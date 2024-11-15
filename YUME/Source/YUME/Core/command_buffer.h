#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Core/definitions.h"



namespace YUME
{
	class YM_API Pipeline;

	class YM_API CommandBuffer
	{
		public:
			virtual ~CommandBuffer() = default;
			virtual void Reset()															= 0;
			virtual void Free()																= 0;
			virtual bool Wait()																= 0;

			virtual bool Init(RecordingLevel p_Level)										= 0;
			virtual void Begin()															= 0;
			virtual void BeginSecondary(const Ref<Pipeline>& p_Pipeline)					= 0;
			virtual void End()																= 0;
			virtual void ExecuteSecondary(CommandBuffer* p_PrimaryCMD)						= 0;
			virtual void Submit()															= 0;

			virtual bool Flush() { return true; }

			virtual RecordingLevel GetRecordingLevel() const								= 0;
			virtual CommandBufferState GetState() const										= 0;

			static Unique<CommandBuffer> Create(const std::string& p_DebugName = "CommandBuffer");
	};
}