#pragma once

#include <memory>

namespace funcc::normalized {
	class IStatement {
	public:
		virtual ~IStatement() = default;
	};

	using PStatement = std::shared_ptr<IStatement>;

	struct File {};
}
