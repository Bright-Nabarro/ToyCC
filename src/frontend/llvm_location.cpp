#include "llvm_location.hpp"
#include <llvm/Support/raw_ostream.h>
#include <cassert>

namespace toycc
{

void LLVMLocation::set_begin(const char* buf)
{
	begin = llvm::SMLoc::getFromPointer(buf);
}

void LLVMLocation::set_end(const char* buf)
{
	end = llvm::SMLoc::getFromPointer(buf);
}

void LLVMLocation::report(Location::DiagKind kind, std::string_view msg) const
{
	assert(begin.isValid());
	assert(m_src_mgr != nullptr && "m_src_mgr uninitialized");
	assert(m_src_mgr->getNumBuffers() > 0);

	auto range = get_range();
	count(kind);
	m_src_mgr->PrintMessage(begin, cvt_kind_to_llvm(kind), msg, range);
}

void LLVMLocation::set_src_mgr(const llvm::SourceMgr* src_mgr) const
{
	m_src_mgr = src_mgr;
}

void LLVMLocation::set_logger(std::shared_ptr<spdlog::async_logger> logger)
{
	m_logger = logger;
}

auto LLVMLocation::get_range() const -> llvm::SMRange
{
	return llvm::SMRange { begin, end };
}

void LLVMLocation::step()
{
	begin = end;
}

void LLVMLocation::update(std::size_t len)
{
	end = llvm::SMLoc::getFromPointer(end.getPointer() + len);
}

auto LLVMLocation::search_counter(Location::DiagKind kind) -> std::size_t
{
	return trace_counter[kind];
}

void LLVMLocation::count(Location::DiagKind kind)
{
	++trace_counter[kind];
}

constexpr
auto LLVMLocation::cvt_kind_to_llvm(Location::DiagKind kind)
	-> llvm::SourceMgr::DiagKind
{
	switch(kind)
	{	
	case Location::dk_error:
		return llvm::SourceMgr::DK_Error;
	case Location::dk_warning:
		return llvm::SourceMgr::DK_Warning;
	case Location::dk_remark:
		return llvm::SourceMgr::DK_Remark;
	case Location::dk_note:
		return llvm::SourceMgr::DK_Note;
	default:
		assert(false && "unkown DiagKind");
	}
}

auto operator<< (std::ostream& os, const LLVMLocation& loc) -> std::ostream&
{
	std::string output_buffer;
	llvm::raw_string_ostream stros { output_buffer };
	loc.m_src_mgr->PrintMessage(stros, loc.begin,
			llvm::SourceMgr::DK_Note, "", loc.get_range());

	os << output_buffer << std::endl;

	return os;
}

}	//namespace toycc
