#include "YUME/yumepch.h"
#include "scene.h"
#include "YUME/Renderer/renderer.h"
#include "entity.h"



namespace YUME
{
	Scene::Scene()
	{
		m_EntityManager = CreateScope<EntityManager>(this);

		m_EntityManager->AddDependency<SpriteComponent, ShapeComponent>();
		m_EntityManager->AddDependency<SpriteComponent, TransformComponent>();
		m_EntityManager->AddDependency<ModelComponent, TransformComponent>();
		m_EntityManager->AddDependency<LightComponent, TransformComponent>();
		m_EntityManager->AddDependency<ShapeComponent, TransformComponent>();
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& p_Tag)
	{
		return m_EntityManager->CreateEntity(p_Tag);
	}

	Entity Scene::CreateEntity(const UUID& p_ID, const std::string& p_Tag)
	{
		return m_EntityManager->CreateEntity(p_ID, p_Tag);
	}

	std::vector<Entity> Scene::GetEntitiesWithTag(const std::string& p_Tag)
	{
		return m_EntityManager->GetEntitiesWithTag(p_Tag);
	}

	Entity Scene::GetEntityByUUID(const UUID& p_ID)
	{
		return m_EntityManager->GetEntityByUUID(p_ID);
	}

	entt::registry& Scene::GetRegistry()
	{
		return m_EntityManager->GetRegistry();
	}

	EntityManager* Scene::GetEntityManager()
	{
		return m_EntityManager.get();
	}

	void Scene::OnUpdate(const Timestep& p_Ts)
	{
		
	}

	void Scene::OnRender()
	{
		Renderer::RenderScene(this);
	}
}
