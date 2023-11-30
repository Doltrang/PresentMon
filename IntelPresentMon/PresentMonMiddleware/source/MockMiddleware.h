#pragma once
#include "Middleware.h"
#include "../../Interprocess/source/Interprocess.h"

namespace pmon::mid
{
	class MockMiddleware : public Middleware
	{
	public:
		MockMiddleware();
		void AdvanceTime(uint32_t milliseconds);
		void Speak(char* buffer) const override;
		const PM_INTROSPECTION_ROOT* GetIntrospectionData() override;
		void FreeIntrospectionData(const PM_INTROSPECTION_ROOT* pRoot) override;
		PM_STATUS OpenSession(uint32_t processId) override { return PM_STATUS_SUCCESS; }
		PM_STATUS CloseSession(uint32_t processId) override { return PM_STATUS_SUCCESS; }
		PM_DYNAMIC_QUERY* RegisterDynamicQuery(std::span<PM_QUERY_ELEMENT> queryElements, double windowSizeMs, double metricOffsetMs) override;
		void FreeDynamicQuery(const PM_DYNAMIC_QUERY* pQuery) override;
		void PollDynamicQuery(const PM_DYNAMIC_QUERY* pQuery, uint8_t* pBlob) override;
		void PollStaticQuery(const PM_QUERY_ELEMENT& element, uint8_t* pBlob) override;
	private:
		uint32_t t = 0;
		std::unique_ptr<ipc::ServiceComms> pServiceComms;
		std::unique_ptr<ipc::MiddlewareComms> pMiddlewareComms;
	};
}