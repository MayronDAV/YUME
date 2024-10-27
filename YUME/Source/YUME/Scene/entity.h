#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"

#include "Component/components.h"
#include "Component/components_2D.h"

#include "scene.h"

#include <entt/entt.hpp>



namespace YUME
{
	template<typename, typename T>
	struct has_OnComponentAdded;

	template<typename Class, typename Func, typename... Args>
	struct has_OnComponentAdded<Class, Func(Args...)>;


	class YM_API Entity
	{
		public:
			Entity() = default;
			Entity(entt::entity p_Handle, Scene* p_Scene);
			Entity(const Entity&) = default;

			template<typename T, typename ...Args>
			T& AddComponent(Args && ...p_Args)
			{
				YM_PROFILE_FUNCTION();

				YM_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
				T& component = m_Scene->GetRegistry().emplace<T>(m_Handle, std::forward<Args>(p_Args)...);
				OnComponentAdded<T>(component);

				return component;
			}

			template<typename T, typename ...Args>
			T& AddOrReplaceComponent(Args && ...p_Args)
			{
				YM_PROFILE_FUNCTION();

				YM_CORE_ASSERT(m_Scene)
				T& component = m_Scene->GetRegistry().emplace_or_replace<T>(m_Handle, std::forward<Args>(p_Args)...);
				OnComponentAdded<T>(component);

				return component;
			}

			template<typename T>
			T& GetComponent()
			{
				YM_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
				return m_Scene->GetRegistry().get<T>(m_Handle);
			}

			template<typename T>
			bool HasComponent()
			{
				YM_CORE_ASSERT(m_Scene)
					return m_Scene->GetRegistry().any_of<T>(m_Handle);
			}

			template<typename T>
			void RemoveComponent()
			{
				YM_PROFILE_FUNCTION();

				YM_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
				m_Scene->GetRegistry().remove<T>(m_Handle);
			}

			void Destroy();

			uint64_t GetID() const;
			Scene* GetScene() const { return m_Scene; }

			bool operator==(const Entity& p_Other) const
			{
				return (m_Handle == p_Other.m_Handle) &&
					(m_Scene == p_Other.m_Scene);
			}

			bool operator!=(const Entity& p_Other) const
			{
				return !(*this == p_Other);
			}

			operator bool() const { return m_Handle != entt::null; }
			operator entt::entity() const { return m_Handle; }
			operator uint32_t() const { return (uint32_t)m_Handle; }

		private:
			template<typename T>
			void OnComponentAdded(T& p_Component)
			{
				YM_PROFILE_FUNCTION();

				OnComponentAddedImpl(p_Component);
			}

			template<typename T>
			void OnComponentAddedImpl(T& p_Component)
			{
				// component T don't have OnComponentAdded
			}

			template<typename T>
			void OnComponentAddedImpl(T& p_Component,
				typename std::enable_if<has_OnComponentAdded<Entity, void(T&)>::value>::type*)
			{
				// component T have OnComponentAdded
				OnComponentAddedImpl<T>(p_Component);
			}

		private:
			entt::entity m_Handle = entt::null;
			Scene* m_Scene = nullptr;
	};
}