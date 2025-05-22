#pragma once

#include "../_external.hh"
#include "ast_common.hh"

namespace funcc::nar {
	class DataType final : public IDeclaration {
		FullIdentifier m_name;
		Range m_nameRange;
		std::vector<IType> m_args;
		std::vector<DataConstructor> m_constructors;

	public:
		DataType(
			Range range,
			FullIdentifier name,
			Range nameRange,
			std::vector<IType>&& args,
			std::vector<DataConstructor>&& constructors
		) :
			IDeclaration{std::move(range), std::move(name), std::move(nameRange), false},
			m_name{std::move(name)},
			m_nameRange{std::move(nameRange)},
			m_args{std::move(args)},
			m_constructors{std::move(constructors)} {}

		~DataType() override = default;

		[[nodiscard]] FullIdentifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<IType> const& GetArgs() const {
			return m_args;
		}

		[[nodiscard]] std::vector<DataConstructor> const& GetConstructors() const {
			return m_constructors;
		}
	};

	class FunctionType final : public IType {
		std::vector<std::shared_ptr<IType>> m_params;
		std::shared_ptr<IType> m_returnType;

	public:
		FunctionType(Range range, std::vector<std::shared_ptr<IType>>&& params, std::shared_ptr<IType> returnType) :
			IType{std::move(range)},
			m_params{std::move(params)},
			m_returnType{std::move(returnType)} {}

		~FunctionType() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IType>> const& GetParams() const {
			return m_params;
		}

		[[nodiscard]] IType const& GetReturnType() const {
			return *m_returnType;
		}
	};

	class NamedType final : public IType {
		Identifier m_name;
		Range m_nameRange;
		std::vector<std::shared_ptr<IType>> m_args;

	public:
		NamedType(Range range, Identifier name, Range nameRange, std::vector<std::shared_ptr<IType>>&& args) :
			IType{std::move(range)},
			m_name{std::move(name)},
			m_nameRange{std::move(nameRange)},
			m_args{std::move(args)} {}

		~NamedType() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IType>> const& GetArgs() const {
			return m_args;
		}
	};

	class NativeType final : public IType {
		std::string m_name;
		Range m_nameRange;
		std::vector<std::shared_ptr<IType>> m_args;

	public:
		NativeType(Range range, std::string name, Range nameRange, std::vector<std::shared_ptr<IType>>&& args) :
			IType{std::move(range)},
			m_name{std::move(name)},
			m_nameRange{std::move(nameRange)},
			m_args{std::move(args)} {}

		~NativeType() override = default;

		[[nodiscard]] std::string const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IType>> const& GetArgs() const {
			return m_args;
		}
	};

	class VarintType final : public IType {
		Identifier m_name;

	public:
		VarintType(Range range, Identifier name) :
			IType{std::move(range)},
			m_name{std::move(name)} {}

		~VarintType() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}
	};

	struct RecordField {
		Identifier name;
		Range nameRange;
		std::shared_ptr<IType> type;
	};

	class RecordType final : public IType {
		std::vector<RecordField> m_fields;

	public:
		RecordType(Range range, std::vector<RecordField>&& fields) :
			IType{std::move(range)},
			m_fields{std::move(fields)} {}

		~RecordType() override = default;

		[[nodiscard]] std::vector<RecordField> const& GetFields() const {
			return m_fields;
		}
	};

	class TupleType final : public IType {
		std::vector<std::shared_ptr<IType>> m_types;

	public:
		TupleType(Range range, std::vector<std::shared_ptr<IType>>&& types) :
			IType{std::move(range)},
			m_types{std::move(types)} {}

		~TupleType() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IType>> const& GetTypes() const {
			return m_types;
		}
	};

	class UnitType final : public IType {
	public:
		UnitType(Range range) :
			IType{std::move(range)} {}

		~UnitType() override = default;
	};

}
