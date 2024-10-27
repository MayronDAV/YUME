#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/uuid.h"

#include <entt/entt.hpp>



namespace YUME
{
	class YM_API Scene;
	class YM_API Entity;

	class YM_API EntityManager
	{
		public:
			EntityManager(Scene* p_Scene);
			
			Entity CreateEntity(const UUID& p_ID, const std::string& p_Tag = "Entity");
			Entity CreateEntity(const std::string& p_Tag = "Entity");

			Entity GetEntityByUUID(const UUID& p_ID);
			bool EntityExists(const UUID& p_ID);

			template <typename... Components>
			auto GetEntitiesWithTypes()
			{
				return m_Registry.group<Components...>();
			}

			template <typename Component, typename Dependency>
			void AddDependency()
			{
				m_Registry.template on_construct<Component>().template connect<&entt::registry::get_or_emplace<Dependency>>();
			}

			entt::registry& GetRegistry() { return m_Registry; }

		private:
			Scene* m_Scene = nullptr;
			entt::registry m_Registry;
	};

} // YUME