#ifndef CFUNCTION_H
#define CFUNCTION_H

#include <functional>
#include "wasm_base.h"
#include "wasm_value.h"

namespace wasm {

namespace detail {

template <class T>
struct is_valid_wasm_value_type:
	std::conditional_t<
		std::is_same_v<T, wasm_sint32_t>
		or std::is_same_v<T, wasm_sint64_t>
		or std::is_same_v<T, wasm_float32_t>
		or std::is_same_v<T, wasm_float64_t>,
		std::true_type,
		std::false_type
	>
{
	
};

template <class T>
inline constexpr const bool is_valid_wasm_value_type_v
	= is_valid_wasm_value_type<T>::value;

} /* namespace detail */

template <class Results, class ... Args>
struct CFunctionImpl;


struct CFunctionWrapperBase
{
	CFunctionWrapperBase(const CFunctionWrapperBase&) = delete;
	CFunctionWrapperBase(CFunctionWrapperBase&&) = delete;

	CFunctionWrapperBase& operator=(const CFunctionWrapperBase&) = delete;
	CFunctionWrapperBase& operator=(CFunctionWrapperBase&&) = delete;

	virtual ~CFunctionWrapperBase() = default;

	virtual const WasmFunctionSignature& signature() const = 0;

	auto return_types() const
	{ return return_types(signature()); }

	auto param_types() const
	{ return param_types(signature()); }

	std::size_t return_count() const
	{ return return_types().size(); }

	std::size_t param_count() const
	{ return param_types().size(); }

	void invoke(
		gsl::span<WasmValue> results, 
		gsl::span<const WasmValue> args
	)
	{
		if(results.size() != return_count())
			throw std::bad_function_call();
		else if(args.size() != param_count())
			throw std::bad_function_call();
		invoke_impl(results, args);
	}

	void invoke(
		gsl::span<TaggedWasmValue> results, 
		gsl::span<const TaggedWasmValue> args
	)
	{
		if(results.size() != return_count())
			throw std::bad_function_call();
		else if(args.size() != param_count())
			throw std::bad_function_call();
		invoke_impl(results, args);
	}


protected:
	CFunctionWrapperBase() = default;
private:
	virtual void invoke_impl(
		gsl::span<WasmValue> results, 
		gsl::span<const WasmValue> args
	) = 0;

	virtual void invoke_impl(
		gsl::span<TaggedWasmValue> results, 
		gsl::span<const TaggedWasmValue> args
	) = 0;

};

template <class InvocableType, class ExposedType>
struct CFunctionWrapperImpl;

template <class ... Results, class ... Args>
struct CFunctionWrapper<std::tuple<Results...> (Args ...)>:
	public CFunctionWrapperBase
{
	static_assert((detail::is_valid_wasm_value_type_v<Results> and ...));
	static_assert((detail::is_valid_wasm_value_type_v<Args> and ...));
	using argument_tuple_type = std::tuple<Args...>;
	using result_type = std::tuple<Results...>;
	using function_type = result_type(Args ...);
	using self_type = CFunctionWrapper<std::tuple<Results...> (Args ...)>;

	template <class InvocableType>
	static constexpr const bool can_store_value 
		= std::is_invocable_r_v<result_type, InvocableType, Args ...>;

	CFunctionWrapper(const CFunctionWrapper&) = delete;
	CFunctionWrapper(CFunctionWrapper&&) = delete;

	CFunctionWrapper& operator=(const CFunctionWrapper&) = delete;
	CFunctionWrapper& operator=(CFunctionWrapper&&) = delete;

	virtual ~CFunctionWrapperImpl() override = default;

	static constexpr std::size_t return_count()
	{ return sizeof...(Results); }
	
	static constexpr std::size_t param_count()
	{ return sizeof...(Args); }
	
	const WasmFunctionSignature& signature() const final override
	{
		if(not signature_)
		{
			// lazily construct the signature
			signature_.emplace(
				{language_type_value_v<Results>...}, 
				{language_type_value_v<Args>...}
			)
		}
		return *signature_;
	}

protected:
	virtual result_type call(Args ... args) = 0;

	CFunctionWrapper() = default;

private:	
	void invoke_impl(gsl::span<WasmValue> results, gsl::span<const WasmValue> args) final override
	{
		assert(results.size() == return_count());
		assert(args.size() == param_count());
		assert(this->base_type::return_count() == return_count());
		assert(this->base_type::param_count() == param_count());
		do_call(
			results,
			args,
			std::make_index_sequence<param_count()>{},
			std::make_index_sequence<return_count()>{}
		);
	}
	
	void invoke_impl(gsl::span<TaggedWasmValue> results, gsl::span<const TaggedWasmValue> args) final override
	{
		assert(results.size() == return_count());
		assert(args.size() == param_count());
		assert(this->base_type::return_count() == return_count());
		assert(this->base_type::param_count() == param_count());
		do_call(
			results,
			args,
			std::make_index_sequence<param_count()>{},
			std::make_index_sequence<return_count()>{}
		);
	}

	template <class T, std::size_t ... ArgIndex, std::size_t ResultIndex>
	void do_call(
		gsl::span<T> results,
		gsl::span<const T> args,
		std::index_sequence<ArgIndex ...>,
		std::index_sequence<ResultIndex ...>
	)
	{
		static_assert(sizeof...(ArgIndex) == param_count());
		static_assert(sizeof...(ResultIndex) == return_count());
		assert(args.size() == param_count());
		assert(results.size() == return_count());
		std::tie(results[ResultIndex].get(tp::member<Results>())...)
			= this->call(args[ArgIndex].get(tp::member<Args>())...);
	}

	// lazily-constructed function signature
	mutable std::optional<const WasmFunctionSignature> signature_;
};

template <class InvocableType, class ... Results, class ... Args>
struct CFunctionWrapperImpl<InvocableType, std::tuple<Results...>(Args...)> final:
	public CFunctionWrapper<std::tuple<Results...>(Args...)>
{
private:
	using result_type = std::tuple<Results...>;
	using exposed_type = result_type(Args...);
	using function_type = exposed_type;
	using base_type = CFunctionWrapper<exposed_type>;
	using self_type = CFunctionWrapperImpl<InvocableType, ExposedType>;

	static constexpr const bool is_valid
		= std::is_invocable_r_v<result_type, InvocableType, Args ...>;
public:
	static_assert(is_valid, "Cannot store given type in CFunctionWrapper.");

	template <class ... Arguments>
	CFunctionWrapperImpl(Arguments&& ... args):
		base_type(),
		value_(std::forward<Arguments>(args) ...)
	{
		
	}

	virtual ~CFunctionWrapperImpl() final override = default;

	CFunctionWrapperImpl(const CFunctionWrapperImpl&) = delete;
	CFunctionWrapperImpl(CFunctionWrapperImpl&&) = delete;

	CFunctionWrapperImpl& operator=(const CFunctionWrapperImpl&) = delete;
	CFunctionWrapperImpl& operator=(CFunctionWrapperImpl&&) = delete;

	result_type call(Args ... args) final override 
	{ return std::invoke(value_, args); }

private:
	InvocableType value_;
};


struct CFunction
{
	CFunction() = delete;

	CFunction(const CFunction&) = default;
	CFunction(CFunction&&) = default;

	CFunction& operator=(const CFunction&) = default;
	CFunction& operator=(CFunction&&) = default;

	void operator()(gsl::span<WasmValue> results, gsl::span<const WasmValue> args)
	{
		if(not impl_)
			throw std::bad_function_call();
		impl_->invoke(results, args);
	}

	void operator()(gsl::span<TaggedWasmValue> results, gsl::span<const TaggedWasmValue> args)
	{
		if(not impl_)
			throw std::bad_function_call();
		impl_->invoke(results, args);
	}

	const WasmFunctionSignature& signature() const
	{ return impl_->signature(); }

	std::size_t return_count() const
	{ return impl_->return_count(); }

	std::size_t param_count() const
	{ return impl_->param_count(); }

private:
	using pointer = std::shared_ptr<CFunctionWrapperBase>;

	template <class WasmFunctionType, class InvocableType>
	friend CFunction make_c_function(InvocableType&& func)
	{
		using wrapper_type = CFunctionWrapperImpl<std::decay_t<InvocableType>, FunctionType>;
		auto ptr = std::make_shared<wrapper_type>(std::forward<InvocableType>(func));
		return CFunction(std::move(ptr));
	}

	template <class F, class I>
	CFunction(std::make_shared<CFunctionWrapperImpl<F, I> p):
		impl_(static_cast<pointer>(p))
	{
		
	}

	pointer impl_;
};


} /* namespace wasm */

#endif /* CFUNCTION_H */
