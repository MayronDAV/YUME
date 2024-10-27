#include "YUME/yumepch.h"
#include "entity.h"
#include "scene.h"



namespace YUME
{
	// Generic template for has_OnComponentAdded
	template<typename, typename T>
	struct has_OnComponentAdded
	{
		static constexpr bool value = false;
	};

	template<typename Class, typename Func, typename... Args>
	struct has_OnComponentAdded<Class, Func(Args...)>
	{
		using type = decltype(check<Class>(0));

		public:
			// type value
			// if std::true_type OnComponentAdded exists
			// if std::false_type OnComponentAdded  don't exists
			static constexpr bool value = type::value;

		private:
			// Checks if an OnComponentAdded function exists
			template<typename T>
			static constexpr auto check(T*) ->
				typename std::is_same<
				decltype(std::declval<T>().OnComponentAdded(std::declval<Args>()...)),
				Func
				>::type;
			// If not exists return std::false_type
			template<typename>
			static constexpr std::false_type check(...) { return {}; }
	};


	Entity::Entity(entt::entity p_Handle, Scene* p_Scene)
		: m_Handle(p_Handle), m_Scene(p_Scene)
	{
		YM_PROFILE_FUNCTION();
	}

	void Entity::Destroy()
	{
		YM_PROFILE_FUNCTION();

		m_Scene->GetRegistry().destroy(m_Handle);
	}

	uint64_t Entity::GetID() const
	{
		YM_PROFILE_FUNCTION();

		return m_Scene->GetRegistry().get<IDComponent>(m_Handle).ID;
	}

} // YUME