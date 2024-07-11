#include "YUME/yumepch.h"
#include "YUME/Core/layer.h"



namespace YUME
{
	Layer::Layer(const std::string& p_Name)
		: m_LayerName(p_Name)
	{
	}

	Layer::~Layer() = default;
}
