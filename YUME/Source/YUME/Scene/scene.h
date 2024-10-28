#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/timestep.h"
#include "YUME/Asset/asset.h"

#include "YUME/Core/reference.h"
#include "YUME/Scene/entity_manager.h"



namespace YUME
{
	class YM_API Entity;

	class YM_API Scene : public Asset
	{
		ASSET_CLASS_TYPE(Scene)

		public:
			Scene();
			Scene(const Scene&) = default;
			~Scene();

			Entity CreateEntity(const std::string& p_Tag = "Entity");
			Entity CreateEntity(const UUID& p_ID, const std::string& p_Tag = "Entity");

			std::vector<Entity> GetEntitiesWithTag(const std::string& p_Tag);
			
			Entity GetEntityByUUID(const UUID& p_ID);

			entt::registry& GetRegistry();
			EntityManager* GetEntityManager();

			void OnUpdate(const Timestep& p_Ts);

			void OnRender();

		private:
			Scope<EntityManager> m_EntityManager;
	};


} // YUME