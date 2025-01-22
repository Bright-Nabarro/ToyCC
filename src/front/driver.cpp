#include "driver.hpp"

#include <format>
#include <llvm/Support/WithColor.h>

namespace tinyc
{

Driver::Driver(llvm::SourceMgr& src_mgr):
	m_ast {},
	m_src_mgr { src_mgr },
	m_bufferid {},
	m_debug_trace { false },
	m_parser {},
	m_location {}
{

}

auto Driver::construct(std::string_view file_name)
	-> std::expected<void, std::string>
{
	auto buffer_or_error = llvm::MemoryBuffer::getFile(file_name);
	if (!buffer_or_error)
	{
		return std::unexpected{std::format("Failed to open {} \n", file_name)};
	}
	m_bufferid = m_src_mgr.AddNewSourceBuffer(std::move(*buffer_or_error), llvm::SMLoc());

	set_flex(get_buffer(),
			 static_cast<int>(
				 m_src_mgr.getMemoryBuffer(m_bufferid)->getBufferSize()));

	const char* buf_str = get_buffer();
	m_location.set_begin(buf_str);
	m_location.set_end(buf_str);
	m_location.set_src_mgr(&m_src_mgr);

	m_parser = std::make_unique<yy::parser>(*this);

	return {};
}

auto Driver::get_buffer() const -> const char*
{
	return m_src_mgr.getMemoryBuffer(m_bufferid)->getBufferStart();
}

auto Driver::parse() -> bool
{
	m_parser->set_debug_level(this->get_trace());
	int parse_ret = (*m_parser)();

	return parse_ret == 0;
}


// 可以设置一个默认location, 每次调用时复制默认
auto Driver::get_location() -> LLVMLocation&
{
	return m_location;
}

auto DriverFactory::produce_driver(std::string_view file_name)
		-> std::expected<std::unique_ptr<Driver>, std::string>
{
	// 构造函数为私有，无法使用std::make_unique
	std::unique_ptr<Driver> driver { new Driver { m_src_mgr } };

	auto void_or_error = driver->construct(file_name);
	if (!void_or_error)
		return std::unexpected(void_or_error.error());

	return driver;
}

}	//namespace tinyc

