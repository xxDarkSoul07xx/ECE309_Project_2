# Design Log — Project 2

(500–800 words total. See spec §5 for what each section must cover.)

## Growth factor and amortized cost
For the Conversation container, I went with doubling the capacity every time we run out of space because it just seemed like the easiest solution to fixing running out of space, so if size equals capacity, the new capacity becomes capacity * 2. If we were starting from 0, the new capacity would become 1.

It is amoritzed O(1) because we do n allocations totally. The reallocations will happen at sizes 1, 2, 4, 8, and so on all the way up to n. The work at each of these reallocations is copying all of the existing elements. This means total copying is 1 + 2 + 4 + ... + n, which is 2n. If we do this across n appends, we get 2n/n = O(1) per append.
## Rule of Five evidence
For the rule of five, we have constructor, destructor, copy constructor, copy assignment, and move constuctor and move assignment.

Constructor: I implemented this by starting with data as nullptr, and sz and cap both as 0. No allocation happens until we get the first append.

Destructor: It calls delete[] on the data. Even if data was nullptr, it's fine because we deleted basically nothing.

Copy constructor: I allocate a brand new buffer of the same capacity and then copy every element one at a time with a for loop. After this is done, the two Conversation objects will have different pointers, so if I decided to destroy one, it wouldn't affect the other one.

Copy assignment: This is the same as the copy constructor, but before copying data, I had to make sure that I deleted the existing buffer and checked for self assignment so that I don't accidentally delete it.

Move constructor: Instead of allocating a new buffer, I grabbed the pointer, size, and the capacity from the other object. Then, I set the other object's pointer to nullptr and sizes to 0. This makes the moved from object empty and in a valid, safe state that won't crash anything or produce some random issues.

Move assignment: I did the same as the mvoe constructor but made sure to delete the existing buffer first and check for self assignment.

## Sentinel scanner: bounded pending_ proof
In the start, pending is empty, so it is holding 0 characters which is less than sentinel.size()-1. On each call to feed we do combined = pending + chunk. Then, we search through combined for the sentinel. If we find it, return everything before it as safe. Otherwise, we figure out what to keep in pending. We will keep the last sentinel.size()-1 characters of the combined as the new pending, and then emit everything before that as safe.

After each feed call, pending will have exactly min(combined.size(), sentinel.size()-1) characters in it. No matter what, it will always be at most sentinel.size()-1.


## What I would change differently
I would probably add an unchecked operator[] for internal use for things like copy constructor loops. Currently, they call the bounds checked at() which throws an error if the index is out of range. However, those loops are already guaranteed to be in bounds by construction, so it means that the check is pretty much useless. An operator[] that just does return data[i] with no check would be better to me.