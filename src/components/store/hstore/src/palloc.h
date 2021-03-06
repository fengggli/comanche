/*
 * (C) Copyright IBM Corporation 2018, 2019. All rights reserved.
 * US Government Users Restricted Rights - Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _COMANCHE_HSTORE_PALLOC_H_
#define _COMANCHE_HSTORE_PALLOC_H_

#include "trace_flags.h"

#include "pobj_bad_alloc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#if defined __clang__
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif
#include <libpmemobj.h> /* PMEMobjpool, PMEMoid, pmemobj_constr */
#pragma GCC diagnostic pop
#if TRACE_PALLOC
#include <iostream> /* cerr */
#endif

#include <cstddef> /* size_t */
#include <tuple>

std::tuple<PMEMoid, std::size_t> palloc_inner(
	PMEMobjpool *pop_
	, std::size_t size_min_
	, std::size_t size_max_
	, uint64_t type_num_
	, pmemobj_constr ctor_
	, void *ctor_arg_
	, const char *
#if TRACE_PALLOC
		use_
#endif
)
{
	PMEMoid oid;
	if ( size_max_ < size_min_ )
	{
		throw pobj_bad_alloc(0, 1, size_max_, size_min_, EINVAL);
	}
	while ( 0 != pmemobj_alloc(pop_, &oid, size_max_, type_num_, ctor_, ctor_arg_) )
	{
		size_max_ = size_max_ / 64U * 63U;
		if ( size_max_ < size_min_ )
		{
			throw pobj_bad_alloc(0, 1, size_max_, size_min_, errno);
		}
	}
#if TRACE_PALLOC
	{
		void *ptr = pmemobj_direct(oid);
		std::cerr << __func__ << " " << use_ << " [" << ptr << ".."
			<< static_cast<void *>(static_cast<char *>(ptr)+size_max_) << ")\n";
	}
#endif
	return std::tuple<PMEMoid, std::size_t>(oid, size_max_);
}

template <typename T>
	PMEMoid palloc(
		PMEMobjpool *pop_
		, std::size_t size_
		, uint64_t type_num_
		, pmemobj_constr ctor_
		, const T &ctor_arg_
		, const char *use_
)
{
	return
		std::get<0>(
			palloc_inner(
				pop_
				, size_
				, size_
				, type_num_
				, ctor_
				, &const_cast<T &>(ctor_arg_)
				, use_
			)
		);
}

static inline PMEMoid palloc(
	PMEMobjpool *pop_
	, std::size_t size_
	, uint64_t type_num_
	, const char *use_
)
{
	return
		std::get<0>(
			palloc_inner(
				pop_
				, size_
				, size_
				, type_num_
				, nullptr
				, nullptr
				, use_
			)
		);
}

static inline std::tuple<PMEMoid, std::size_t> palloc(
	PMEMobjpool *pop_
	, std::size_t size_min_
	, std::size_t size_mac_
	, uint64_t type_num_
	, const char *use_
)
{
	return
		palloc_inner(
			pop_
			, size_min_
			, size_mac_
			, type_num_
			, nullptr
			, nullptr
			, use_
		);
}

void zfree(
	PMEMoid oid
	, const char *
#if TRACE_PALLOC
		why
#endif
)
{
#if TRACE_PALLOC
	{
		const auto ptr = pmemobj_direct(oid);
		std::cerr << __func__ << " " << why << " [" << ptr << "..)\n";
	}
#endif
	pmemobj_free(&oid);
}

#endif
