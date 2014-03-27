//
//  DPPacket.h
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

#ifndef __DPPACKET_H__
#define __DPPACKET_H__

#include <Rayne/Rayne.h>

namespace DP
{
	class Packet : public RN::Object
	{
	public:
		enum class Type : int32
		{
			RequestWorld,
			AnswerWorld,
			RequestTransform,
			AnswerTransform,
			RequestSceneNode,
			AnswerSceneNode,
			RequestDeleteSceneNode,
			AnswerDeleteSceneNode
		};
		
		static Packet *WithType(Type type);
		static Packet *WithTypeAndData(Type type, const void *data, size_t length);
		static Packet *WithTypeAndSerializer(Type type, RN::Serializer *serializer);
		
		Packet(Type type, const void *data, size_t length);
		Packet(RN::Deserializer *deserializer);
		~Packet() override;
		
		void Serialize(RN::Serializer *serializer) override;
		
		Type GetType() const { return _type; }
		size_t GetLength() const { return _length; }
		void GetData(void *ptr) const;
		RN::Deserializer *GetDeserializer() const;
		
	private:
		Type _type;
		
		uint8 *_data;
		size_t _length;
		
		RNDeclareMeta(Packet)
	};
}

#endif /* __DPPACKET_H__ */
