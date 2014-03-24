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
		_isRemoteChange(false)
	{
		_lightClass  = RN::Light::MetaClass();
		_cameraClass = RN::Camera::MetaClass();
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		RN_ASSERT(!enet_initialize(), "Enet could not be initialized!");
		
		RN::Timer::ScheduledTimerWithDuration(std::chrono::milliseconds(5), [&](){
			if(!_isConnected)
			{
				return;
			}
			
			if(_isServer)
				StepServer();
			else
				StepClient();
		}, true);
		
		RN::MessageCenter::GetSharedInstance()->AddObserver(kDPWorkspaceSelectionChanged, [this](RN::Message *message) {
			
			RN::SafeRelease(_sceneNodes);
			_sceneNodes = RN::SafeRetain(static_cast<RN::Array *>(message->GetObject()));
			
		}, this);
	}
	
	WorldAttachment::~WorldAttachment()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		DestroyHost();
		enet_deinitialize();
		
		RN::SafeRelease(_sceneNodes);
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void WorldAttachment::SetCamera(RN::Camera *camera)
	{
		_camera = camera;
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
		if(!node)// || !(changeSet & RN::SceneNode::ChangeSet::Position))
			return;
		
		if(!_isConnected)
			return;
		
		if(_isRemoteChange)
		{
			_isRemoteChange = false;
			return;
		}
		
		if(node->IsKindOfClass(RN::Camera::MetaClass()))
			return;
		
		if(node->IsKindOfClass(DP::Gizmo::MetaClass()))
			return;
		
		if(_isServer)
		{
			RN::FlatSerializer serializer;
			std::string request("answerTransforms");
			serializer.EncodeString(request);
			serializer.EncodeInt64(node->GetLID());
			serializer.EncodeVector3(node->GetWorldPosition());
			serializer.EncodeVector3(node->GetWorldScale());
			serializer.EncodeQuarternion(node->GetWorldRotation());
			SendDataToAll(serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
		}
		else
		{
			RN::FlatSerializer serializer;
			std::string request("requestTransforms");
			serializer.EncodeString(request);
			serializer.EncodeInt64(node->GetLID());
			serializer.EncodeVector3(node->GetWorldPosition());
			serializer.EncodeVector3(node->GetWorldScale());
			serializer.EncodeQuarternion(node->GetWorldRotation());
			SendDataToServer(serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
			
			TransformRequest transformrequest;
			transformrequest.lid = node->GetLID();
			transformrequest.position = node->GetWorldPosition();
			transformrequest.scale = node->GetWorldScale();
			transformrequest.rotation = node->GetWorldRotation();
			
			_requestedTransforms.push_back(transformrequest);
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
	
	void WorldAttachment::ApplyTransforms(uint64 lid, const RN::Vector3 &position, const RN::Vector3 &scale, const RN::Quaternion &rotation)
	{
		TransformRequest transformrequest;
		transformrequest.lid = lid;
		transformrequest.position = position;
		transformrequest.scale = scale;
		transformrequest.rotation = rotation;
		auto it = std::find(_requestedTransforms.begin(), _requestedTransforms.end(), transformrequest);
		if(it != _requestedTransforms.end())
		{
			_requestedTransforms.erase(it);
			return;
		}
		
		RN::Array *nodes = RN::World::GetActiveWorld()->GetSceneNodes();
		nodes->Enumerate<RN::SceneNode>([&](RN::SceneNode *node, uint32 i, bool &stop){
			if(node->GetLID() == lid)
			{
				_isRemoteChange = true;
				node->SetWorldPosition(position);
				_isRemoteChange = true;
				node->SetWorldScale(scale);
				_isRemoteChange = true;
				node->SetWorldRotation(rotation);
				
				stop = true;
			}
		});
	}
	
	void WorldAttachment::RequestSceneNode(RN::Object *object, const RN::Vector3 &position)
	{
		if(_isServer || !_isConnected)
		{
			RN::SceneNode *node = CreateSceneNode(object, position);
			
			RN::FlatSerializer serializer;
			std::string request("answerSceneNode");
			serializer.EncodeString(request);
			serializer.EncodeObject(node);
			SendDataToAll(serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
		}
		else
		{
			RN::FlatSerializer serializer;
			std::string request("requestSceneNode");
			serializer.EncodeString(request);
			serializer.EncodeObject(object);
			serializer.EncodeVector3(position);
			SendDataToServer(serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
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
					printf("A new client connected from %x:%u.\n",
						   event.peer->address.host,
						   event.peer->address.port);
					/* Store any relevant client information here. */
					event.peer->data = new std::string("Client information");
					break;
				}
				
				case ENET_EVENT_TYPE_RECEIVE:
				{
					RN::Data *data = new RN::Data(event.packet->data, event.packet->dataLength);
					RN::FlatDeserializer *deserializer = new RN::FlatDeserializer(data);
					enet_packet_destroy(event.packet);
					data->Release();
					std::string received(deserializer->DecodeString());
					printf("%s\n", received.c_str());
					
					if(received.compare("requestWorld") == 0)
					{
						RN::FlatSerializer serializer;
						RN::WorldCoordinator::GetSharedInstance()->SaveWorld(&serializer);
						SendDataToClient(event.peer, "answerWorld", 12);
						SendDataToClient(event.peer, serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
					}
					else if(received.compare("requestSceneNode") == 0)
					{
						RN::Object *object = deserializer->DecodeObject();
						RN::Vector3 position = deserializer->DecodeVector3();
						RequestSceneNode(object, position);
					}
					else if(received.compare("requestTransforms") == 0)
					{
						uint64 lid = deserializer->DecodeInt64();
						RN::Vector3 position = deserializer->DecodeVector3();
						RN::Vector3 scale = deserializer->DecodeVector3();
						RN::Quaternion rotation = deserializer->DecodeQuaternion();
						
						RN::FlatSerializer serializer;
						std::string request("answerTransforms");
						serializer.EncodeString(request);
						serializer.EncodeInt64(lid);
						serializer.EncodeVector3(position);
						serializer.EncodeVector3(scale);
						serializer.EncodeQuarternion(rotation);
						SendDataToAll(serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
						
						ApplyTransforms(lid, position, scale, rotation);
					}
					
					deserializer->Release();
					
					break;
				}
					
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("%s disconected.\n", event.peer->data);
					/* Reset the peer's client information. */
					event.peer->data = NULL;
					break;
				}
					
				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}
	}
	
	extern void ActivateDownpour();
	extern void DeactivateDownpour();
	
	static const char *__DPCookie = "__DPCookie";
	
	void WorldAttachment::StepClient()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		ENetEvent event;
		while(enet_host_service(_host, &event, 1) > 0)
		{
			switch(event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					printf("A new client connected from %x:%u.\n",
						   event.peer->address.host,
						   event.peer->address.port);
					/* Store any relevant client information here. */
					event.peer->data = new std::string("Client information");
					break;
				}
					
				case ENET_EVENT_TYPE_RECEIVE:
				{
					if(event.packet->dataLength == 12)
					{
						std::string received(reinterpret_cast<const char *>(event.packet->data));
						if(received.compare("answerWorld") == 0)
						{
							printf("%s\n", received.c_str());
							ENetEvent newevent;
							if(enet_host_service(_host, &newevent, 20000))
							{
								RN::Data *data = new RN::Data(newevent.packet->data, newevent.packet->dataLength);
								RN::FlatDeserializer *deserializer = new RN::FlatDeserializer(data);
								enet_packet_destroy(newevent.packet);
								data->Release();
								
								RN::Kernel::GetSharedInstance()->ScheduleFunction([deserializer]() {
									
									{
										RN::World::GetActiveWorld()->RemoveAttachment(WorldAttachment::GetSharedInstance());
										RN::AutoreleasePool pool;
										DeactivateDownpour();
									}
									
									RN::Kernel::GetSharedInstance()->ScheduleFunction([deserializer]() {
										
										RN::MessageCenter::GetSharedInstance()->AddObserver(kRNWorldCoordinatorDidFinishLoadingMessage, [](RN::Message *message) {
											
											ActivateDownpour();
											RN::MessageCenter::GetSharedInstance()->RemoveObserver(const_cast<char *>(__DPCookie));
											
										}, const_cast<char *>(__DPCookie));
										
										RN::WorldCoordinator::GetSharedInstance()->LoadWorld(deserializer);
										deserializer->Release();
									});
								});
							}
							break;
						}
					}
					
					RN::Data *data = new RN::Data(event.packet->data, event.packet->dataLength);
					RN::FlatDeserializer *deserializer = new RN::FlatDeserializer(data);
					enet_packet_destroy(event.packet);
					data->Release();
					std::string received(deserializer->DecodeString());
					printf("%s\n", received.c_str());
					
					if(received.compare("answerSceneNode") == 0)
					{
						deserializer->DecodeObject();
					}
					else if(received.compare("answerTransforms") == 0)
					{
						uint64 lid = deserializer->DecodeInt64();
						RN::Vector3 position = deserializer->DecodeVector3();
						RN::Vector3 scale = deserializer->DecodeVector3();
						RN::Quaternion rotation = deserializer->DecodeQuaternion();
						
						ApplyTransforms(lid, position, scale, rotation);
					}
					
					deserializer->Release();
					
					break;
				}
					
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("%s disconected.\n", event.peer->data);
					/* Reset the peer's client information. */
					event.peer->data = NULL;
					break;
				}

					
				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}
	}

	
	void WorldAttachment::CreateServer()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		DestroyHost();
		
		ENetAddress address;
		
		/* Bind the server to the default localhost.     */
		/* A specific host address can be specified by   */
		/* enet_address_set_host (& address, "x.x.x.x"); */
		address.host = ENET_HOST_ANY;
		
		/* Bind the server to port 1234. */
		address.port = 5555;
		
		_host = enet_host_create(&address /* the address to bind the server host to */,
								 32      /* allow up to 32 clients and/or outgoing connections */,
								 2      /* allow up to 2 channels to be used, 0 and 1 */,
								 0      /* assume any amount of incoming bandwidth */,
								 0      /* assume any amount of outgoing bandwidth */);
		
		RN_ASSERT(_host, "Enet could not create server!");
		
		_isServer = true;
		_isConnected = true;
	}
	
	void WorldAttachment::CreateClient()
	{
		RN::LockGuard<decltype(_lock)> lock(_lock);
		DestroyHost();
		
		_host = enet_host_create(NULL /* create a client host */,
								 1 /* only allow 1 outgoing connection */,
								 2 /* allow up 2 channels to be used, 0 and 1 */,
								 0 /* assume any amount of incoming bandwidth */,
								 0 /* assume any amount of outgoing bandwidth */);
		
		RN_ASSERT(_host, "Enet could not create client!");
		
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
		_isConnected = false;
	}
	
	void WorldAttachment::Connect()
	{
		if(!_host || _isServer)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		ENetAddress address;
		ENetEvent event;
		
		/* Connect to some.server.net:1234. */
		enet_address_set_host(&address, "localhost");
		address.port = 5555;
		
		/* Initiate the connection, allocating the two channels 0 and 1. */
		_peer = enet_host_connect(_host, &address, 2, 0);
		RN_ASSERT(_peer, "Enet could not create a peer!");
		
		/* Wait up to 5 seconds for the connection attempt to succeed. */
		if(enet_host_service(_host, &event, 5000) > 0 &&
		   event.type == ENET_EVENT_TYPE_CONNECT)
		{
			_isConnected = true;
			
			RN::FlatSerializer serializer;
			std::string request("requestWorld");
			serializer.EncodeString(request);
			SendDataToServer(serializer.GetSerializedData()->GetBytes(), serializer.GetSerializedData()->GetLength());
		}
		else
		{
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			_isConnected = false;
		}
	}
	
	void WorldAttachment::Disconnect()
	{
		if(!_peer || _isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		ENetEvent event;
		enet_peer_disconnect(_peer, 0);
		
		_isConnected = false;
		
		/* Allow up to 3 seconds for the disconnect to succeed
		 * and drop any packets received packets.
		 */
		while(enet_host_service(_host, & event, 3000) > 0)
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
		
		/* We've arrived here, so the disconnect attempt didn't */
		/* succeed yet.  Force the connection down.             */
		enet_peer_reset(_peer);
		_peer = nullptr;
	}
	
	void WorldAttachment::SendDataToServer(const void *data, uint32 length)
	{
		if(!_isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		ENetPacket *packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(_peer, 0, packet);
	}
	
	void WorldAttachment::SendDataToClient(ENetPeer *peer, const void *data, uint32 length)
	{
		if(!_isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		ENetPacket *packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);
	}
	
	void WorldAttachment::SendDataToAll(const void *data, uint32 length)
	{
		if(!_isConnected)
			return;
		
		RN::LockGuard<decltype(_lock)> lock(_lock);
		
		ENetPacket *packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
		enet_host_broadcast(_host, 0, packet);
	}

}
