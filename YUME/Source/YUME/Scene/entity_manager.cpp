#include "YUME/yumepch.h"
#include "entity_manager.h"
#include "scene.h"
#include "entity.h"



namespace YUME
{
	EntityManager::EntityManager(Scene* p_Scene)
		: m_Scene(p_Scene)
	{
	}

	Entity EntityManager::CreateEntity(const UUID& p_ID, const std::string& p_Tag)
	{
		YM_PROFILE_FUNCTION();

		Entity entt = { m_Registry.create(), m_Scene };
		m_Registry.emplace<IDComponent>(entt, p_ID);
		m_Registry.emplace<TagComponent>(entt, p_Tag);

		return entt;
	}


	Entity EntityManager::CreateEntity(const std::string& p_Tag)
	{
		return CreateEntity(UUID(), p_Tag);
	}

	Entity EntityManager::GetEntityByUUID(const UUID& p_ID)
	{
		YM_PROFILE_FUNCTION();
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			auto& idComponent = m_Registry.get<IDComponent>(entity);
			if (idComponent.ID == p_ID)
				return Entity(entity, m_Scene);
		}

		YM_CORE_WARN("Entity not found by ID");
		return Entity{};
	}

	bool EntityManager::EntityExists(const UUID& p_ID)
	{
		YM_PROFILE_FUNCTION();

		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			auto& idComponent = m_Registry.get<IDComponent>(entity);
			if (idComponent.ID == p_ID)
				return true;
		}

		return false;
	}

	std::vector<Entity> EntityManager::GetEntitiesWithTag(const std::string& p_Tag)
	{
		std::vector<Entity> filteredEntities;
		m_Registry.view<TagComponent>().each([&](auto p_Entity, const TagComponent& p_TC)
		{
			if (p_TC.Tag == p_Tag)
			{
				filteredEntities.push_back(Entity{ p_Entity, m_Scene });
			}
		});

		return filteredEntities;
	}

}
