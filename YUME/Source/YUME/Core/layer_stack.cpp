#include "YUME/yumepch.h"
#include "YUME/Core/layer_stack.h"


namespace YUME
{
	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* p_Layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, p_Layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* p_Overlay)
	{
		m_Layers.emplace_back(p_Overlay);
	}

	void LayerStack::PopLayer(Layer* p_Layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex,p_Layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			p_Layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* p_Overlay)
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), p_Overlay);
		if (it != m_Layers.end())
		{
			p_Overlay->OnDetach();
			m_Layers.erase(it);
		}
	}

}

