//
//  DPWorldAttachment.cpp
//  Downpour
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "DPWorldAttachment.h"
#include "DPWorkspace.h"
#include "DPEditorIcon.h"
#include "DPInfoPanel.h"

namespace DP
{
	RNDefineSingleton(WorldAttachment)
	
	WorldAttachment::WorldAttachment() :
		_sceneNodes(nullptr),
		_camera(nullptr),
		_host(nullptr),
		_peer(nullptr),
		_isConnected(false),
		_isServer(false),
		_isRemoteChange(false),
		_isLoadingWorld(false)
	{
		_lightClass  = RN::Light::MetaClass();
		_cameraClass = RN::Camera::MetaClass();
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		RN_ASSERT(enet_initialize() == 0, "Enet could not be initialized!");
		
		RN::Timer::ScheduledTimerWithDuration(std::chrono::milliseconds(5), [&]() {
			if(!_isConnected)
				return;
			
			_isServer ? StepServer() : StepClient();
		}, true);
	}
	
	WorldAttachment::~WorldAttachment()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		DestroyHost();
		enet_deinitialize();
		
		RN::SafeRelease(_sceneNodes);
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void WorldAttachment::Activate(RN::Camera *camera)
	{
		_camera = camera;
		
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			
			RN::SafeRelease(_sceneNodes);
			_sceneNodes = RN::SafeRetain(static_cast<RN::Array *>(message->GetObject()));
			
		}, this);
	}
	
	void WorldAttachment::DidBeginCamera(RN::Camera *camera)
	{
		if(!_camera)
			return;
		
		if(_camera == camera && _sceneNodes)
		{
			_sceneNodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				if(node->IsKindOfClass(_cameraClass))
					return;
				
				if(node->IsKindOfClass(_lightClass))
				{
					float distance = _camera->GetWorldPosition().GetDistance(node->GetWorldPosition());
					float tessellation = ((_camera->GetClipFar() - distance) / _camera->GetClipFar()) * 30.0f;
					
					tessellation = std::max(5.0f, tessellation);
					
					RN::Debug::DrawSphere(node->GetBoundingSphere(), RN::Color::Yellow(), static_cast<int>(floorf(tessellation)));
				}
				else
				{
					RN::Debug::DrawBox(node->GetBoundingBox(), RN::Color::Red());
				}
				
			});
		}
	}
	
	
	void WorldAttachment::DidAddSceneNode(RN::SceneNode *node)
	{
		RN::MessageCenter::GetSharedInstance()->PostMessage(kDPWorldAttachmentDidAddSceneNode, node, nullptr);
	}
	void WorldAttachment::WillRemoveSceneNode(RN::SceneNode *node)
	{
		RN::MessageCenter::GetSharedInstance()->PostMessage(kDPWorldAttachmentWillRemoveSceneNode, node, nullptr);
	}
	
	void WorldAttachment::SceneNodeDidUpdate(RN::SceneNode *node, RN::SceneNode::ChangeSet changeSet)
	{
		if(!(changeSet & RN::SceneNode::ChangeSet::Position))
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		if(!_isConnected || _isLoadingWorld || _isRemoteChange)
			return;
		
		if(node->GetFlags() & RN::SceneNode::Flags::NoSave)
			return;
		
		if(node->IsKindOfClass(RN::Camera::MetaClass()))
			return;
		
		if(node->IsKindOfClass(DP::Gizmo::MetaClass()))
			return;
		
		if(node->IsKindOfClass(DP::EditorIcon::MetaClass()))
			return;
		
		if(_sceneNodeLookup.count(node->GetLID()) == 0)
			return;
		
		if(_isServer)
		{
			TransformRequest request;
			request.lid      = node->GetLID();
			request.position = node->GetPosition();
			request.scale    = node->GetScale();
			request.rotation = node->GetRotation();
			
			BroadcastPacket(Packet::WithTypeAndData(Packet::Type::AnswerTransform, &request, sizeof(TransformRequest)));
		}
		else
		{
			TransformRequest request;
			request.lid      = node->GetLID();
			request.position = node->GetPosition();
			request.scale    = node->GetScale();
			request.rotation = node->GetRotation();
			
			SendPacketToServer(Packet::WithTypeAndData(Packet::Type::RequestTransform, &request, sizeof(TransformRequest)));
			_requestedTransforms.push_back(std::move(request));
		}
	}
	
	
	
	bool operator== (const WorldAttachment::TransformRequest &first, const WorldAttachment::TransformRequest &second)
	{
		if(first.lid != second.lid)
			return false;
		if(!(first.position == second.position))
			return false;
		if(!(first.scale == second.scale))
			return false;
		if(!(first.rotation == second.rotation))
			return false;
		
		return true;
	}
	
	void WorldAttachment::ApplyTransforms(const TransformRequest &request)
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		auto it = std::find(_requestedTransforms.begin(), _requestedTransforms.end(), request);
		if(it != _requestedTransforms.end())
		{
			_requestedTransforms.erase(it);
			return;
		}
		
		RN_ASSERT(_sceneNodeLookup.count(request.lid) > 0, "A transform for a not existing ID has been received!");
		
		RN::SceneNode *node = _sceneNodeLookup[request.lid];
		if(node)
		{
			_isRemoteChange = true;
			
			node->SetPosition(request.position);
			node->SetScale(request.scale);
			node->SetRotation(request.rotation);
			
			_isRemoteChange = false;
		}
	}
	
	void WorldAttachment::RequestSceneNode(RN::Object *object, const RN::Vector3 &position)
	{
		if(_isServer || !_isConnected)
		{
			RN::SceneNode *node = CreateSceneNode(object, position);
			if(node)
			{
				RegisterSceneNodeRecursive(node);
				
				RN::FlatSerializer *serializer = new RN::FlatSerializer();
				serializer->EncodeObject(node);
				
				BroadcastPacket(Packet::WithTypeAndSerializer(Packet::Type::AnswerSceneNode, serializer));
				
				serializer->Release();
			}
		}
		else
		{
			RN::FlatSerializer *serializer = new RN::FlatSerializer();
			serializer->EncodeObject(object);
			serializer->EncodeVector3(position);
			
			SendPacketToServer(Packet::WithTypeAndSerializer(Packet::Type::RequestSceneNode, serializer));
			
			serializer->Release();
		}
	}
	
	RN::SceneNode *WorldAttachment::CreateSceneNode(RN::Object *object, const RN::Vector3 &position)
	{
		if(object->IsKindOfClass(RN::Model::MetaClass()))
		{
			// Place the model
			RN::Model *model = static_cast<RN::Model *>(object);
			RN::Entity *entity = new RN::Entity(model, position);
			
			// Get the world to register the new entity immediately
			RN::World::GetActiveWorld()->ApplyNodes();
			Workspace::GetSharedInstance()->SetSelection(entity);
			
			return entity;
		}
		
		if(object->IsKindOfClass(RN::Value::MetaClass()))
		{
			RN::Value *value = static_cast<RN::Value *>(object);
			
			try
			{
				RN::MetaClassBase *meta = value->GetValue<RN::MetaClassBase *>();
				if(meta && meta->InheritsFromClass(RN::SceneNode::MetaClass()))
				{
					RN::SceneNode *node = static_cast<RN::SceneNode *>(meta->Construct());
					node->SetPosition(position);
					
					RN::World::GetActiveWorld()->ApplyNodes();
					
					return node;
				}
			}
			catch(RN::Exception e)
			{} // Meh...
		}
		
		return nullptr;
	}
	
	void WorldAttachment::DuplicateSceneNodes(RN::Array *sceneNodes)
	{
		if(!_isConnected || _isServer)
		{
			std::vector<uint64> ids;
			
			RN::Array *duplicates = new RN::Array();
			RN::Serializer *serializer = new RN::FlatSerializer();
			
			sceneNodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				RN::MetaClassBase *meta = node->Class();
				bool noDirectCopy = false;
				
				// Find the first class that supports copying to avoid trying to make a copy
				// of something that doesn't support copying in the first place
				while(!meta->SupportsCopying())
				{
					noDirectCopy = true;
					meta = meta->SuperClass();
				}
				
				try
				{
					RN::SceneNode *copy = static_cast<RN::SceneNode *>(meta->ConstructWithCopy(node));
					ids.push_back(node->GetLID());
					RegisterSceneNodeRecursive(copy);
					duplicates->AddObject(copy);
					
					if(noDirectCopy)
						RNDebug("Can't copy %s, copying %s instead (make sure to implement the Copyable meta class trait!", node->Class()->Name().c_str(), meta->Name().c_str());
				}
				catch(RN::Exception e)
				{} // Meh...
			});
			
			if(_isServer && !ids.empty())
			{
				serializer->EncodeBytes(ids.data(), ids.size() * sizeof(uint64));
				serializer->EncodeObject(duplicates);
				BroadcastPacket(Packet::WithTypeAndSerializer(Packet::Type::AnswerDuplicateSceneNode, serializer));
			}
			
			RN::World::GetActiveWorld()->ApplyNodes();
			duplicates->Release();
			serializer->Release();
		}
		else
		{
			std::vector<uint64> ids;
			
			sceneNodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				if(_sceneNodeLookup.count(node->GetLID()))
				{
					ids.push_back(node->GetLID());
				}
				
			});
			
			if(!ids.empty())
				BroadcastPacket(Packet::WithTypeAndData(Packet::Type::RequestDuplicateSceneNode, ids.data(), ids.size() * sizeof(uint64)));
		}
	}
	
	void WorldAttachment::DeleteSceneNodes(RN::Array *sceneNodes)
	{
		if(!_isConnected || _isServer)
		{
			std::vector<uint64> ids;
			
			sceneNodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				if(_sceneNodeLookup.count(node->GetLID()))
				{
					UnregisterSceneNodeRecursive(node);
					ids.push_back(node->GetLID());
				}
				
				if(node->GetParent())
					node->RemoveFromParent();
				
				node->RemoveFromWorld();
			});
			
			if(_isServer && !ids.empty())
				BroadcastPacket(Packet::WithTypeAndData(Packet::Type::AnswerDeleteSceneNode, ids.data(), ids.size() * sizeof(uint64)));
		}
		else
		{
			std::vector<uint64> ids;
			
			sceneNodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t index, bool &stop) {
				
				if(_sceneNodeLookup.count(node->GetLID()))
				{
					ids.push_back(node->GetLID());
				}
				
			});
			
			if(!ids.empty())
				BroadcastPacket(Packet::WithTypeAndData(Packet::Type::RequestDeleteSceneNode, ids.data(), ids.size() * sizeof(uint64)));
		}
	}
	
	void WorldAttachment::HandleSceneNodeDeletion(const std::vector<uint64> &ids)
	{
		for(uint64 id : ids)
		{
			auto iterator = _sceneNodeLookup.find(id);
			if(iterator != _sceneNodeLookup.end())
			{
				RN::SceneNode *node = iterator->second;
				UnregisterSceneNodeRecursive(node);
				
				if(node->GetParent())
					node->RemoveFromParent();
				
				node->RemoveFromWorld();
			}
		}
	}
	
	void WorldAttachment::RegisterSceneNodeRecursive(RN::SceneNode *node)
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		_sceneNodeLookup[node->GetLID()] = node;
		node->GetChildren()->Enumerate<RN::SceneNode>([&](RN::SceneNode *n, size_t i, bool &end){RegisterSceneNodeRecursive(n);});
	}
	
	void WorldAttachment::UnregisterSceneNodeRecursive(RN::SceneNode *node)
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		auto iterator = _sceneNodeLookup.find(node->GetLID());
		if(iterator != _sceneNodeLookup.end())
		{
			_sceneNodeLookup.erase(iterator);
		}
		
		node->GetChildren()->Enumerate<RN::SceneNode>([&](RN::SceneNode *n, size_t i, bool &end){UnregisterSceneNodeRecursive(n);});
	}
	
	
	void WorldAttachment::StepServer()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		ENetEvent event;
		while(enet_host_service(_host, &event, 1) > 0)
		{
			switch(event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					break;
				}
				
				case ENET_EVENT_TYPE_RECEIVE:
				{
					Packet *packet;
					
					{
						RN::Data *data = new RN::Data(event.packet->data, event.packet->dataLength);
						RN::FlatDeserializer *deserializer = new RN::FlatDeserializer(data->Autorelease());
						
						packet = static_cast<Packet *>(deserializer->DecodeObject());
						deserializer->Release();
						
						enet_packet_destroy(event.packet);
					}
					
					switch(packet->GetType())
					{
						case Packet::Type::RequestWorld:
						{
							RN::FlatSerializer *serializer = new RN::FlatSerializer();
							RN::WorldCoordinator::GetSharedInstance()->SaveWorld(serializer);
							
							SendPacketToPeer(event.peer, Packet::WithTypeAndSerializer(Packet::Type::AnswerWorld, serializer));
							serializer->Release();
							
							break;
						}
							
						case Packet::Type::RequestSceneNode:
						{
							RN::Deserializer *deserializer = packet->GetDeserializer();
							
							RN::Object *object   = deserializer->DecodeObject();
							RN::Vector3 position = deserializer->DecodeVector3();
							
							RequestSceneNode(object, position);
							break;
						}
						
						case Packet::Type::RequestTransform:
						{
							TransformRequest request;
							packet->GetData(&request);
							
							ApplyTransforms(request);
							BroadcastPacket(Packet::WithTypeAndData(Packet::Type::AnswerTransform, &request, sizeof(TransformRequest)));
							
							break;
						}
							
						case Packet::Type::RequestDuplicateSceneNode:
						{
							size_t count = packet->GetLength() / sizeof(uint64);
							std::vector<uint64> ids(count);
							
							packet->GetData(ids.data());
							RN::Array *nodes = new RN::Array();
							for(auto i : ids)
							{
								if(_sceneNodeLookup.count(i) > 0)
									nodes->AddObject(_sceneNodeLookup[i]);
							}
							
							DuplicateSceneNodes(nodes);
							break;
						}
							
						case Packet::Type::RequestDeleteSceneNode:
						{
							size_t count = packet->GetLength() / sizeof(uint64);
							std::vector<uint64> ids(count);
							
							packet->GetData(ids.data());
							HandleSceneNodeDeletion(ids);
							
							BroadcastPacket(Packet::WithTypeAndData(Packet::Type::AnswerDeleteSceneNode, ids.data(), ids.size() * sizeof(uint64)));
							break;
						}
							
						default:
							break;
					}
					
					break;
				}
					
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					break;
				}
					
				default:
					break;
			}
		}
	}
	
	extern void ActivateDownpour();
	extern void DeactivateDownpour();
	
	void WorldAttachment::StepClient()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		ENetEvent event;
		
		while(enet_host_service(_host, &event, 1) > 0)
		{
			switch(event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
				{
					Packet *packet;
					
					{
						RN::Data *data = new RN::Data(event.packet->data, event.packet->dataLength);
						RN::FlatDeserializer *deserializer = new RN::FlatDeserializer(data->Autorelease());
						
						packet = static_cast<Packet *>(deserializer->DecodeObject());
						deserializer->Release();
						
						enet_packet_destroy(event.packet);
					}
					
					switch(packet->GetType())
					{
						case Packet::Type::AnswerWorld:
						{
							RN::Deserializer *deserializer = packet->GetDeserializer()->Retain();
							
							RN::Kernel::GetSharedInstance()->ScheduleFunction([this, deserializer]() {
								
								{
									RN::World::GetActiveWorld()->RemoveAttachment(this);
									RN::AutoreleasePool pool;
									DeactivateDownpour();
								}
								
								RN::Kernel::GetSharedInstance()->ScheduleFunction([this, deserializer]() {
									
									RN::MessageCenter::GetSharedInstance()->AddObserver(kRNWorldCoordinatorDidFinishLoadingMessage, [this](RN::Message *message) {
										
										_sceneNodeLookup.clear();
										
										RN::Array *nodes = RN::World::GetActiveWorld()->GetSceneNodes();
										nodes->Enumerate<RN::SceneNode>([](RN::SceneNode *node, size_t i, bool &stop ){
											if(!node->IsKindOfClass(RN::Camera::MetaClass()))
											{
												WorldAttachment::GetSharedInstance()->_sceneNodeLookup[node->GetLID()] = node;
											}
										});
										
										RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
										ActivateDownpour();
										RN::World::GetActiveWorld()->Update(0.0f);
										
										_isLoadingWorld = false;
										
									}, this);
									
									RN::WorldCoordinator::GetSharedInstance()->LoadWorld(deserializer);
									deserializer->Release();
								});
							});
							
							break;
						}
							
						case Packet::Type::AnswerSceneNode:
						{
							RN::Deserializer *deserializer = packet->GetDeserializer();
							
							RN::SceneNode *node = static_cast<RN::SceneNode *>(deserializer->DecodeObject());
							
							RegisterSceneNodeRecursive(node);
							break;
						}
							
						case Packet::Type::AnswerTransform:
						{
							TransformRequest request;
							packet->GetData(&request);
							
							ApplyTransforms(request);
							break;
						}
							
						case Packet::Type::AnswerDuplicateSceneNode:
						{
							RN::Deserializer *deserializer = packet->GetDeserializer();
							size_t size;
							void *data = deserializer->DecodeBytes(&size);
							size_t count = size / sizeof(uint64);
							RN::Array *nodes = static_cast<RN::Array *>(deserializer->DecodeObject());
							
							nodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, size_t i, bool &end){
								RegisterSceneNodeRecursive(node);
							});
							
							break;
						}
							
						case Packet::Type::AnswerDeleteSceneNode:
						{
							size_t count = packet->GetLength() / sizeof(uint64);
							std::vector<uint64> ids(count);
							
							packet->GetData(ids.data());
							HandleSceneNodeDeletion(ids);
							break;
						}
							
						default:
							break;
					}
					
					break;
				}
					
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					//_isConnected = false;
					break;
				}

				default:
					break;
			}
		}
	}

	
	void WorldAttachment::CreateServer()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		DestroyHost();
		
		ENetAddress address { .host = ENET_HOST_ANY, .port = 2003 };
		RN_ASSERT((_host = enet_host_create(&address, 32, 2, 0, 0)), "Enet couldn't create server");
		
		_isServer    = true;
		_isConnected = true;
		
		RN::Array *nodes = RN::World::GetActiveWorld()->GetSceneNodes();
		nodes->Enumerate<RN::SceneNode>([](RN::SceneNode *node, size_t i, bool &stop) {
			if(!node->IsKindOfClass(RN::Camera::MetaClass()))
			{
				WorldAttachment::GetSharedInstance()->_sceneNodeLookup[node->GetLID()] = node;
			}
		});
	}
	
	void WorldAttachment::CreateClient()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		DestroyHost();
		
		RN_ASSERT((_host = enet_host_create(NULL, 1, 2, 0, 0)), "Enet couldn't create client!");
		_isServer = false;
	}
	
	void WorldAttachment::DestroyHost()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		Disconnect();
		
		if(_host)
			enet_host_destroy(_host);
		
		_host = nullptr;
		_peer = nullptr;
	}
	
	void WorldAttachment::Connect(const std::string &ip)
	{
		if(!_host || _isServer)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		ENetAddress address;
		ENetEvent event;
		
		/* Connect to some.server.net:1234. */
		enet_address_set_host(&address, ip.c_str());
		address.port = 2003;
		
		/* Initiate the connection, allocating the two channels 0 and 1. */
		RN_ASSERT((_peer = enet_host_connect(_host, &address, 2, 0)), "Enet couldn't create a peer!");
		
		/* Wait up to 5 seconds for the connection attempt to succeed. */
		if(enet_host_service(_host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
		{
			_isConnected = true;
			SendPacketToServer(Packet::WithType(Packet::Type::RequestWorld));
		}
		else
		{
			_isConnected = false;
			InfoPanel::WithMessage(RNCSTR("Couldn't connect to server! Ping time out"));
		}
	}
	
	void WorldAttachment::Disconnect()
	{
		if(!_peer || !_isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		ENetEvent event;
		
		enet_peer_disconnect(_peer, 0);
		_isConnected = false;
		
		while(enet_host_service(_host, &event, 3000) > 0)
		{
			switch(event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
					enet_packet_destroy(event.packet);
					break;
					
				case ENET_EVENT_TYPE_DISCONNECT:
					_peer = nullptr;
					return;
					
				default:
					break;
			}
		}
		
		enet_peer_reset(_peer);
		_peer = nullptr;
	}
	
	
	void WorldAttachment::SendPacketToServer(Packet *packet)
	{
		SendPacketToPeer(_peer, packet);
	}
	
	void WorldAttachment::SendPacketToPeer(ENetPeer *peer, Packet *packet)
	{
		if(!_isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		// Encode the packet
		RN::Data *data;
		{
			RN::FlatSerializer *serializer = new RN::FlatSerializer();
			serializer->EncodeObject(packet);
			
			data = serializer->GetSerializedData();
			serializer->Release();
		}
		
		ENetPacket *enetPacket = enet_packet_create(data->GetBytes(), data->GetLength(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, enetPacket);
	}
	
	void WorldAttachment::BroadcastPacket(Packet *packet)
	{
		if(!_isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		// Encode the packet
		RN::Data *data;
		{
			RN::FlatSerializer *serializer = new RN::FlatSerializer();
			serializer->EncodeObject(packet);
			
			data = serializer->GetSerializedData();
			serializer->Release();
		}
		
		ENetPacket *enetPacket = enet_packet_create(data->GetBytes(), data->GetLength(), ENET_PACKET_FLAG_RELIABLE);
		enet_host_broadcast(_host, 0, enetPacket);
	}
}
