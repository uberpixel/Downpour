//
//  DPPacket.cpp
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

#include "DPPacket.h"

namespace DP
{
	RNDefineMeta(Packet, RN::Object)
	
	Packet::Packet(Type type, const void *data, size_t length) :
		_type(type),
		_data(nullptr),
		_length(0)
	{
		if(data)
		{
			_data = new uint8[length];
			_length = length;
			
			const uint8 *source = reinterpret_cast<const uint8 *>(data);
			std::copy(source, source + _length, _data);
		}
	}
	
	Packet::Packet(RN::Deserializer *deserializer)
	{
		_type = static_cast<Type>(deserializer->DecodeInt32());
		_data = nullptr;
		
		bool hasData = deserializer->DecodeBool();
		if(hasData)
		{
			const uint8 *source = reinterpret_cast<const uint8 *>(deserializer->DecodeBytes(&_length));
			
			_data = new uint8[_length];
			
			std::copy(source, source + _length, _data);
		}
	}
	
	Packet::~Packet()
	{
		delete [] _data;
	}
	
	void Packet::Serialize(RN::Serializer *serializer)
	{
		serializer->EncodeInt32(static_cast<int32>(_type));
		
		if(_data)
		{
			serializer->EncodeBool(true);
			serializer->EncodeBytes(_data, _length);
		}
		else
		{
			serializer->EncodeBool(false);
		}
	}
	
	
	Packet *Packet::WithType(Type type)
	{
		Packet *packet = new Packet(type, nullptr, 0);
		return packet->Autorelease();
	}
	
	Packet *Packet::WithTypeAndData(Type type, const void *data, size_t length)
	{
		Packet *packet = new Packet(type, data, length);
		return packet->Autorelease();
	}
	
	Packet *Packet::WithTypeAndSerializer(Type type, RN::Serializer *serializer)
	{
		RN::Data *data = serializer->GetSerializedData();
		return WithTypeAndData(type, data->GetBytes(), data->GetLength());
	}
	
	void Packet::GetData(void *ptr) const
	{
		if(_data)
		{
			uint8 *destination = reinterpret_cast<uint8 *>(ptr);
			std::copy(_data, _data + _length, destination);
		}
	}
	
	RN::Deserializer *Packet::GetDeserializer() const
	{
		RN::Data *data = RN::Data::WithBytes(_data, _length);
		RN::FlatDeserializer *deserializer = new RN::FlatDeserializer(data);
		return deserializer->Autorelease();
	}
}
