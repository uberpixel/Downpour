//
//  DPWorldAttachment.h
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

#ifndef __DPWORLDATTACHMENT_H__
#define __DPWORLDATTACHMENT_H__

#include <Rayne/Rayne.h>
#include <enet/enet.h>
#include "DPPacket.h"

#define kDPWorldAttachmentDidAddSceneNode     RNCSTR("kDPWorldAttachmentDidAddSceneNode")
#define kDPWorldAttachmentWillRemoveSceneNode RNCSTR("kDPWorldAttachmentWillRemoveSceneNode")

namespace DP
{
	class WorldAttachment : public RN::WorldAttachment, public RN::ISingleton<WorldAttachment>
	{
	public:
		struct TransformRequest
		{
		public:
			friend bool operator== (const TransformRequest &first, const TransformRequest &second);
			
			uint64 lid;
			RN::Vector3 position;
			RN::Vector3 scale;
			RN::Quaternion rotation;
		};
		
		WorldAttachment();
		~WorldAttachment();
		
		void Activate(RN::Camera *camera);
		
		void DidBeginCamera(RN::Camera *camera) override;
		
		void DidAddSceneNode(RN::SceneNode *node) override;
		void WillRemoveSceneNode(RN::SceneNode *node) override;
		
		void SceneNodeDidUpdate(RN::SceneNode *node, RN::SceneNode::ChangeSet changeSet) override;
		
		void RequestSceneNode(RN::Object *object, const RN::Vector3 &position);
		RN::SceneNode *CreateSceneNode(RN::Object *object, const RN::Vector3 &position);
		void DeleteSceneNodes(RN::Array *sceneNodes);
		void ApplyTransforms(const TransformRequest &request);
		
		void SelectSceneNode(RN::SceneNode *node);
		
		void StepServer();
		void StepClient();
		
		void CreateServer();
		void CreateClient();
		void DestroyHost();
		
		void Connect(const std::string &ip);
		void Disconnect();
		
		
		void SendPacketToServer(Packet *packet);
		void SendPacketToPeer(ENetPeer *peer, Packet *packet);
		void BroadcastPacket(Packet *packet);
		
		bool IsServer() const { return _isServer; }
		bool IsConnected() const { return _isConnected; }
		
	private:
		void HandleSceneNodeDeletion(const std::vector<uint64> &ids);
		void RegisterSceneNodeRecursive(RN::SceneNode *node);
		void UnregisterSceneNodeRecursive(RN::SceneNode *node);
		
		RN::Array *_sceneNodes;
		RN::Camera *_camera;
		
		ENetHost *_host;
		ENetPeer *_peer;
		
		bool _isConnected;
		bool _isServer;
		bool _isRemoteChange;
		bool _isLoadingWorld;
		
		std::vector<TransformRequest> _requestedTransforms;
		std::unordered_map<uint64, RN::SceneNode*> _sceneNodeLookup;
		
		RN::RecursiveSpinLock _lock;
		
		RN::MetaClassBase *_lightClass;
		RN::MetaClassBase *_cameraClass;
		
		RNDeclareSingleton(WorldAttachment);
	};
}

#endif /* __DPWORLDATTACHMENT_H__ */
