#ifndef UTILITIES_SEGMENT_TRACKER_H
#define UTILITIES_SEGMENT_TRACKER_H



class SegmentTracker
{
	struct Range: public std::pair<std::size_t, std::size_t>
	{
		using std::pair<std::size_t, std::size_t>::pair;
		const std::size_t& begin() const { return first; }
		std::size_t& begin() { return first; }
		const std::size_t& end() const { return second; }
		std::size_t& end() { return second; }
		bool try_add_before(const Range& other)
		{
			if(first == other.last)
			{
				first = other.first;
				return true;
			}
			else if(first > other.last)
				throw std::out_of_range("Overlapping ranges are not permitted.");
			else
				return false;
		}

		bool try_add_after(const Range& other)
		{
			if(last == other.first)
			{
				last = other.last;
				return true;
			}
			else if(last < other.first)
				throw std::out_of_range("Overlapping ranges are not permitted.");
			else
				return false;
		}
	};
	
	void insert_range(std::size_t index, std::size_t length)
	{
		Range range(index, index + length);
		auto pos = find_range(range);
		if(pos < ranges.end())
		{
			// Merged with the position found by binary search
			bool pos_added = pos->try_add_before(range);
			// Merged with the position just before the one found by binary search
			bool prev_added = (pos > ranges.begin()) and (std::prev(pos)->try_add_after(range));
			if(pos_added and prev_added)
			{
				// If this range filled in the gap perfectly
				// we'll have to merge the two existing ranges into one
				std::prev(pos)->second = pos->second;
				ranges.erase(pos);
			}
			else if(not (pos_added or prev_added))
			{
				// Otherwise if this range doesn't line up exactly
				// with either of the existing ranges, we just insert it
				ranges.insert(pos, range);
			}
			// If neither of those branches were taken, then one of the two 
			// already-existing ranges was able to merge itself with the given range
		}
		else if((ranges.size() == 0) or (not ranges.back().try_add_after(range)))
		{
			// Either this is the first range being added, or
			// or it didn't mesh perfectly with the already-existing last range.
			// Either way, the given range gets inserted at the end
			ranges.push_back(range);
		}
	}
	auto find_range(const Range& range) const
	{
		return std::lower_bound(ranges.begin(), ranges.end(), range);
	}

	std::size_t count() const
	{ return ranges.size(); }
	
	auto begin() const
	{ return ranges.cbegin(); }
	
	auto end() const
	{ return ranges.cend(); }
private:
	std::vector<Range> ranges;
};


#endif /* UTILITIES_SEGMENT_TRACKER_H */
