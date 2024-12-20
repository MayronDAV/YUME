#pragma once

#include "YUME/Core/base.h"
#include "YUME/Events/event.h"
#include "YUME/Core/timestep.h"


namespace YUME
{
	class YM_API Layer
	{
		public:
			explicit Layer(const std::string& p_Name = "Layer");
			virtual ~Layer();

			virtual void OnAttach() {}
			virtual void OnDetach() {}
			virtual void OnUpdate(const Timestep& p_Ts) {}
			virtual void OnImGuiRender() {}
			virtual void OnEvent(Event& event) {}

			inline const std::string& GetName() const { return m_LayerName; }

		protected:
			std::string m_LayerName;
	};

}