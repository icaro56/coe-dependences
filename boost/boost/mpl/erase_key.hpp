
#ifndef BOOST_MPL_ERASE_KEY_HPP_INCLUDED
#define BOOST_MPL_ERASE_KEY_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: erase_key.hpp 85956 2013-09-26 13:05:50Z skelly $
// $Date: 2013-09-26 15:05:50 +0200 (Do, 26. Sep 2013) $
// $Revision: 85956 $

#include <boost/mpl/erase_key_fwd.hpp>
#include <boost/mpl/sequence_tag.hpp>
#include <boost/mpl/aux_/erase_key_impl.hpp>
#include <boost/mpl/aux_/na_spec.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>

namespace boost { namespace mpl {

template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename BOOST_MPL_AUX_NA_PARAM(Key)
    >
struct erase_key
    : erase_key_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,Key >
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(2,erase_key,(Sequence,Key))
};

BOOST_MPL_AUX_NA_SPEC(2,erase_key)

}}

#endif // BOOST_MPL_ERASE_KEY_HPP_INCLUDED
