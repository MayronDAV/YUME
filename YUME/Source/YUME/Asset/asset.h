#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/uuid.h"



namespace YUME
{
	using AssetHandle = UUID;

	enum class AssetType
	{
		Unknown = 0,
		Scene, Texture2D
	};

	#define ASSET_CLASS_TYPE(type)													\
		public:																		\
			static AssetType GetStaticType() { return AssetType::type; }			\
			virtual AssetType GetType() const override { return GetStaticType(); }	\
			virtual const char* GetTypeName() const override { return #type; }

	class YM_API Asset
	{
		public:
			AssetHandle Handle;

			virtual AssetType GetType() const = 0;
			virtual const char* GetTypeName() const = 0;
	};

} // YUME