#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"


class Sandbox : public YUME::Application
{
	public:
		Sandbox() = default;
		~Sandbox() override = default;
};


YUME::Application* YUME::CreateApplication()
{
	return new Sandbox();
}