//
// UniqueAccessExpireStrategy.h
//
// $Id$
//
// Library: Foundation
// Package: Cache
// Module:  UniqueAccessExpireStrategy
//
// Definition of the UniqueAccessExpireStrategy class.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef  Foundation_UniqueAccessExpireStrategy_INCLUDED
#define  Foundation_UniqueAccessExpireStrategy_INCLUDED

#include "Poco/KeyValueArgs.h"
#include "Poco/ValidArgs.h"
#include "Poco/AbstractStrategy.h"
#include "Poco/Bugcheck.h"
#include "Poco/Timestamp.h"
#include "Poco/EventArgs.h"
#include "Poco/UniqueExpireStrategy.h"
#include <set>
#include <map>
#include <stdio.h>

namespace Poco
{
	template <class TKey, class TValue>
	class UniqueAccessExpireCache;
	
	template <class TKey, class TValue>
	class UniqueAccessExpireStrategy : public Poco::UniqueExpireStrategy <TKey, TValue>
	{
		public:
			typedef std::map<TKey, SharedPtr<TValue > > 	DataHolder;
			typedef typename DataHolder::iterator       	DataIterator;

			typedef std::multimap<Poco::Timestamp, TKey>	TimeIndex;
			typedef typename TimeIndex::iterator			IndexIterator;
			typedef typename TimeIndex::const_iterator		ConstIndexIterator;
			typedef std::map<TKey, IndexIterator>			Keys;
			typedef typename Keys::iterator					Iterator;

			UniqueAccessExpireStrategy (const Poco::UniqueAccessExpireCache<TKey, TValue>* cache)
				: _cache (cache)
			{
			}
			
			virtual void onAdd(const void*, const KeyValueArgs <TKey, TValue>& args)
			{
				const Timestamp& expire = args.value().getExpiration();
				Timestamp now;
				IndexIterator it = this->_keyIndex.insert(std::make_pair(now + (expire.epochMicroseconds () * 1000), args.key()));
				std::pair<Iterator, bool> stat = this->_keys.insert(std::make_pair(args.key(), it));
				if (!stat.second)
				{
					this->_keyIndex.erase(stat.first->second);
					stat.first->second = it;
				}
			}

			virtual void onGet (const void*, const TKey& key)
			{
				Iterator it = this->_keys.find (key);
				if (it != this->_keys.end ()) {
					DataIterator itd = _cache->_data.find (key);
					if (itd != _cache->_data.end ()) {
						this->_keyIndex.erase(it->second); 
						const Timestamp& expire = itd->second->getExpiration ();
						Poco::Timestamp now;
						IndexIterator itIdx = this->_keyIndex.insert (typename TimeIndex::value_type (now + (expire.epochMicroseconds () * 1000), key));
						it->second = itIdx;
					}
				}
			}

		protected:
			const Poco::UniqueAccessExpireCache<TKey, TValue>* _cache;
	};
} // namespace Poco

#endif
