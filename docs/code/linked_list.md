# File: `linked_list.h`

## Classes

- `ListElement`
- `LinkedList`
- `LinkedListIterator`

## Functions

- `void		SetData ( const TYPE& d )		    { data = d ; } // Quagmire - this may look like a mem leak, but dont change it, this behavior is expected where it's called`
- `void		SetLastNext ( ListElement<TYPE>* p )`
- `void		SetNext (ListElement<TYPE>* n)	{ next = n ; }`
- `void		SetPrev (ListElement<TYPE>* p)	{ prev = p ; }`
- `void          ReplaceData(const TYPE&);`
- `void Append (const TYPE&);`
- `void Insert (const TYPE&);`
- `TYPE Pop();`
- `TYPE PeekTop();`
- `void Clear();`
- `void LCount() { count--; }`
- `void ResetCount() { count=0; }`
- `int32	Count() { return count; }`
- `void Advance();`
- `bool IsFirst()`
- `bool IsLast()`
- `bool MoreElements();`
- `void MoveFirst();`
- `void MoveLast();`
- `void RemoveCurrent(bool DeleteData = true);`
- `void Replace(const TYPE& new_data);`
- `void Reset();`
- `void SetDir(direction);`
- `void LinkedListIterator<TYPE>::Advance()`
- `bool LinkedListIterator<TYPE>::MoreElements()`
- `void LinkedListIterator<TYPE>::MoveFirst()`
- `void LinkedListIterator<TYPE>::MoveLast()`
- `void LinkedListIterator<TYPE>::RemoveCurrent(bool DeleteData)`
- `void LinkedListIterator<TYPE>::Replace(const TYPE& new_data)`
- `void LinkedListIterator<TYPE>::Reset()`
- `void LinkedListIterator<TYPE>::SetDir(direction d)`
- `void ListElement<TYPE>::ReplaceData(const TYPE& new_data)`
- `void LinkedList<TYPE>::Clear() {`
- `void LinkedList<TYPE>::Append(const TYPE& data)`
- `void LinkedList<TYPE>::Insert(const TYPE& data)`
- `TYPE LinkedList<TYPE>::Pop() {`
- `TYPE LinkedList<TYPE>::PeekTop() {`

## Notable Comments

- /*
- */
- //			if (current_element == 0)
- //			{
- //				return;
- //			}
- //  if (prev != 0)
- //  {
- //  }
- //  if (next != 0)
