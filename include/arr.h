/**
 * @file include/arr.h
 * Declares array types and helper methods.
 * @copyright Copyright (c) 2014 Yasser Asmi; Released under the MIT
 *            License (http://opensource.org/licenses/MIT)
 */

#ifndef _ARR_H
#define _ARR_H

#include "util.h"
#include "var.h"
#include "str.h"

namespace jvar
{

/**
 * BArray is a base class and maintains a contiguous chunk of memory as an dynamic array.
 * It can also work with fixed memory.  It allows finding items and inserting them in
 * order using binary search.  Unless there is a special need, use ObjArray class instead
 * of using this directly.
 */
class BArray
{
public:

    /**
     * A comparison callback for sorting.
     */
    typedef int (*Compare)(const void*, const void*);

    /**
     * Constructor
     *
     * @param  elemsize Size of the array element
     * @param  comp     Compare function
     */
    BArray(size_t elemsize, Compare comp) :
        mMemPtr(NULL),
        mElemSize((int)elemsize),
        mMaxLen(0),
        mComp(comp),
        mCountLocal(0),
        mFlags(0)
    {
        mCountPtr = &mCountLocal;
    }
    ~BArray()
    {
    }
    /**
     * Constructor
     *
     * @param  src Another array
     */
    inline BArray(const BArray& src)
    {
        copyFrom((BArray&)src, false, false);
    }
    /**
     * Constructor
     *
     * @param  src Another array
     */
    inline BArray(BArray& src)
    {
        copyFrom(src, false, false);
    }

    /**
     * Copy \p src into this array.
     */
    inline BArray& operator=(const BArray& src)
    {
        copyFrom((BArray&)src, false, false);
        return *this;
    }

    /**
     * Copy \p src into this array.
     */
    inline BArray& operator=(const BArray* src)
    {
        copyFrom((BArray&)*src, false, false);
        return *this;
    }

    /**
     * Enters fixed memory mode using the provided memory
     *
     * @param memptr   Pointer to allocated fixed memory
     * @param countptr Pointer to a variable where current count is held
     * @param maxlen   Maximum number of elements (size specified in constructor)
     */
    void useFixedMem(void* memptr, int* countptr, int maxlen);

    /**
     * Deletes all elements
     */
    void clear();

    /**
     * Insert an element into the array
     *
     * @param pos  Position to insert at
     * @param elem Pointer to element which will be copied
     */
    void* insert(int pos, const void* elem);

    /**
     * Appends or adds an element at the end of the array
     *
     * @param elem Pointer to the element which will be copied
     *
     * @return     Pointer to the element in array
     */
    inline void* append(const void* elem)
    {
        return insert(length(), elem);
    }

    /**
     * Adds an element in order using the compare function
     *
     * @param elem Pointer to the element which will be copied
     *
     * @return     Pointer to the element in array
     */
    inline void* add(const void* elem)
    {
        return addOrModify(elem, false);
    }

    /**
     * Adds an element or modifies it if it already exists
     *
     * @param elem        Pointer to element which will be copied
     * @param modifyfound Should it be modified (true=allow, false=don't allow)
     *
     * @return            Pointer to the element in array
     */
    void* addOrModify(const void* elem, bool modifyfound = true);

    /**
     * Removes an element from the array
     *
     * @param  pos Position of the item to remove
     *
     * @return     Success
     */
    bool remove(int pos);

    /**
     * Removes an element from array that matches the provided element
     * @param  elem Pointer to an element to search
     * @return      Success
     */
    bool remove(const void* elem);

    /**
     * Finds an element in array that matches the provided element
     *
     * @param elem Pointer to an element to search
     *
     * @return     Pointer to the element in array
     */
    void* find(const void* elem);

    /**
     * Retrieves an element from the array.  Note: This function does not do any
     * bounds checking.
     *
     * @param pos Position to retrieve the element
     */
    inline void* get(int pos)
    {
        return (char*)mMemPtr + pos * mElemSize;
    }

    /**
     * Returns the number of elements in the array
     *
     * @return Number of elements
     */
    inline int length()
    {
        return *mCountPtr;
    }

    /**
     * Determines if the array is at capacity.
     *
     * @return True if full, false otherwise
     */
    inline bool full()
    {
        return (*mCountPtr >= mMaxLen);
    }

    /**
     * Sorts the array
     *
     * @param comp Compare function (if NULL, the one from constructor is used)
     */
    inline void sort(Compare comp = NULL)
    {
        if (comp == NULL)
        {
            comp = mComp;
        }
        assert(comp);
        qsort(mMemPtr, *mCountPtr, mElemSize, comp);
    }

    /**
     * Finds the position of an element
     *
     * @param  findelem Pointer to an element to search
     * @param  pos      Returns the position of the found element
     *
     * @return          True if found, false otherwise
     */
    inline bool findPos(const void* findelem, int& pos)
    {
        return binSearch(findelem, pos);
    }

    /**
     * Ensures that the array has at least the specified allocation
     *
     * @param elemcount Number of element to reserve
     */
    inline void reserve(int elemcount)
    {
        if (elemcount > mMaxLen)
        {
            ensureAlloc(elemcount);
        }
    }

private:
    void* mMemPtr;
    int mElemSize;
    int* mCountPtr;
    int mMaxLen;
    Compare mComp;
    int mCountLocal;
    Buffer mBuf;

public:
    /**
     * Flags for this array
     */
    uint mFlags;

public:

    /**
     * List of flags used in \ref mFlags.
     */
    enum
    {
        /**
         * Array is currently using fixed buffer provided via useFixedMemory
         */
        FLAG_FIXEDBUF = 0x1
    };

protected:

    /**
     * Copy data from \ref src to this array.
     * @param src The array to read from.
     * @param alloconly Only allocate memory, don't copy anything.
     * @param move Move elements, don't copy them.
     */
    void copyFrom(BArray& src, bool alloconly, bool move);

    /**
     * Implements a binary search.
     *
     * @param findelem The element to find.
     * @param[out] pos The position of the element.
     * @return true if the element was found.
     */
    bool binSearch(const void* findelem, int& pos);

    /**
     * Ensure that enough memory for the array is allocated.
     */
    void ensureAlloc(int desiredlen);


};

/**
 * Iter class template is used to iterate over an array as follows:
 * \code
 *    for (Iter<Obj> i; arr.forEach(i); )
 *    {
 *        foo(i->field);
 *    }
 * \endcode
 */
template <class T>
class Iter
{
public:
    Iter() :
        mPos(-1),
        mObj(NULL),
        mKey(NULL)
    {
    }
    /**
     * @return   Returns a reference to the current element
     */
    inline T* operator->()
    {
        return mObj;
    }
    /**
     * @return   Dereferences the current element
     */
    inline T& operator*()
    {
        return *mObj;
    }
    /**
     * @return   Returns a pointer to the current element
     */
    inline T* operator&()
    {
        return mObj;
    }
    /**
     * @return Returns the position of of the current element in the iterator
     */
    inline int pos()
    {
        return mPos;
    }
    /**
     * @return Returns the key for the current element if available
     */
    inline const char* key()
    {
        return mKey;
    }

public:
/** \cond INTERNAL */
    int mPos;
    T* mObj;
    const char* mKey;
/** \endcond */
};


/**
 * ObjArray is similar to std::vector.  It maintains a contiguous chunk of memory as an dynamic
 * array of objects.  It takes care of calling constructors and destructors.  It allows finding
 * elements and inserting them in order using binary search.  It allows creating objects
 * directly on the array as well.
 */
template <class T>
class ObjArray : public BArray
{
public:
    /**
     * Construct a blank ObjArray.
     */
    inline ObjArray() :
        BArray(sizeof(T), NULL),
        mSupportImpl(NULL)
    {
    }

    /**
     * Construct a blank ObjArray with \p as the comparison operator.
     */
    inline ObjArray(Compare comp) :
        BArray(sizeof(T), comp),
        mSupportImpl(NULL)
    {
    }

    /**
     * Construct an ObjArray and copy the data from \p src into it.
     */
    inline ObjArray(const ObjArray& src) :
        BArray(sizeof(T), NULL),
        mSupportImpl(NULL)
    {
        copyFrom((ObjArray&)src);
    }

    /**
     * Construct an ObjArray and copy the data from \p src into it.
     */
    inline ObjArray(ObjArray& src) :
        BArray(sizeof(T), NULL),
        mSupportImpl(NULL)
    {
        copyFrom(src);
    }

    /**
     * Copy the data from \p src into this array.
     */
    inline ObjArray& operator=(const ObjArray& src)
    {
        copyFrom((ObjArray&)src);
        return *this;
    }

    /**
     * Copy the data from \p src into this array.
     */
    inline ObjArray& operator=(const ObjArray* src)
    {
        copyFrom(*src);
        return *this;
    }

    ~ObjArray()
    {
        // Call the destructor for all objects in the array.

        for (int i = 0; i < BArray::length(); i++)
        {
            T* obj = (T*)BArray::get(i);
            obj->~T();
        }

        delete mSupportImpl;
    }

    /**
     * Inserts a new element at the provided position in the array
     *
     * @param  pos Position to insert
     *
     * @return     Pointer to the newly created element
     */
    inline T* insert(int pos)
    {
        T* obj = (T*)BArray::insert(pos, NULL);

        // Inplace new the object using default constructor.

        return new(obj) T();
    }

    /**
     * Inserts an element at the provided position in the array.  The new element is not
     * constructed by ths call.  The caller uses "inplace new" to call a non-default constructor.
     *
     * @param  pos Position to insert
     *
     * @return     Pointer to the inserted element
     */
    inline T* insertCustom(int pos)
    {

        return (T*)BArray::insert(pos, NULL);
    }

    /**
     * Appends a new element at the end of the array
     *
     * @return Pointer to the newly created element
     */
    inline T* append()
    {
        T* obj = (T*)BArray::append(NULL);

        // Inplace new the object using default constructor.

        return new(obj) T();
    }

    /**
     * Adds a new element or modifies it if exists.
     *
     * @param  keyelem     Element to search and add if doesn't exist or modify
     * @param  modifyfound Allow modifying the lement
     *
     * @return             Pointer to the added or modified element
     */
    T* addOrModify(const T* keyelem, bool modifyfound = true)
    {
        int pos;

        if (BArray::binSearch(keyelem, pos))
        {
            if (modifyfound)
            {
                return get(pos);
            }
            else
            {
                return NULL;
            }
        }

        T* obj = (T*)BArray::insert(pos, NULL);

        // Inplace new the object using copy constructor.

        return new(obj) T(*keyelem);
    }

    /**
     * Adds an item based to the array keeping it sorted
     *
     * @param  keyelem Element to add
     *
     * @return         Pointer to the added or modifed element
     */
    inline T* add(const T* keyelem)
    {
        return addOrModify(keyelem, false);
    }

    /**
     * Removes an item using a key
     *
     * @param  keyelem Element to remove (only key is used)
     *
     * @return         Success
     */
    inline bool removeKey(const T* keyelem)
    {
        int pos;
        if (!binSearch(keyelem, pos))
        {
            return false;
        }
        return remove(pos);
    }

    /**
     * Removes an element from the provided position
     *
     * @param  pos Position to remove the item
     *
     * @return     Success
     */
    inline bool remove(int pos)
    {
        T* obj = get(pos);
        if (obj == NULL)
        {
            return NULL;
        }

        // Call the destructor.

        obj->~T();

        // Delete the item from the array.

        return BArray::remove(pos);
    }

    /**
     * Finds the item
     *
     * @param  elem Element to find (only the key is used)
     *
     * @return      Pointer to the element
     */
    inline T* find(const T* elem)
    {
        return (T*)BArray::find(elem);
    }

    /**
     * Returns an element from the array
     *
     * @param  pos Position of retrieve the element
     *
     * @return     Pointer to the element
     */
    inline T* get(int pos)
    {
        // Bounds check.

        if ((uint)pos >= length())
        {
            return NULL;
        }
        return (T*)BArray::get(pos);
    }

    /**
     * Returns an iterator to go over elements of the array in order
     *
     * @param  iter Iterator
     *
     * @return      Success
     */
    bool forEach(Iter<T>& iter)
    {
        iter.mPos++;
        if (iter.mPos < length())
        {
            iter.mObj = get(iter.mPos);

            // Key is not available

            return true;
        }
        return false;
    }

    /**
     * Returns an iterator go over elements in reverse order
     *
     * @param  iter Iterator
     *
     * @return      Success
     */
    bool forEachReverse(Iter<T>& iter)
    {
        if (iter.mPos == -1)
        {
            iter.mPos = length();
        }
        iter.mPos--;
        if (iter.mPos >= 0)
        {
            iter.mObj = get(iter.mPos);
            // Key is not available
            return true;
        }
        return false;
    }

/** \cond INTERNAL */
    inline InterfaceImpl* setSupportImpl(InterfaceImpl* impl)
    {
        delete mSupportImpl;
        mSupportImpl = impl;
        return mSupportImpl;
    }

    inline InterfaceImpl* getSupportImpl()
    {
        return mSupportImpl;
    }
/** \endcond */

protected:
/** \cond INTERNAL */
    InterfaceImpl* mSupportImpl;
/** \endcond */

private:

    void copyFrom(ObjArray& src)
    {
        // Init the base array, alloc memory but do not copy the elements

        BArray::copyFrom(src, true, false);

        // Call the copy constructors

        for (int i = 0; i < BArray::length(); i++)
        {
            T* obj = (T*)BArray::get(i);
            T* srcobj = (T*)src.get(i);

            // Inplace new the object using its copy constructor.

            new(obj) T(*srcobj);
        }

        // Make a copy of the support interface.

        if (src.mSupportImpl)
        {
            delete mSupportImpl;
            mSupportImpl = src.mSupportImpl->newImpl();
        }
    }
};


/**
 * PropArray is similar to stl::map.  It maintains a key value relationship.  Keys are
 * character strings.  Value is the value provided in the template.
 */
template <class T>
class PropArray
{
public:
    PropArray() :
        mData(NULL),
        mIndex(NULL),
        mSupp(NULL)
    {
        mData.reserve(INITSIZE);
        mIndex.reserve(INITSIZE);
    }

    ~PropArray()
    {
    }

    /**
     * Adds or modifies a property and sets it value
     *
     * @param  keyname     Property key name
     * @param  modifyfound Allow modification if found
     *
     * @return             Pointer to the value or NULL if not allowed to modify
     */
    T* addOrModify(const char* keyname, bool modifyfound = true)
    {
        // Find the item in the index.
        int pos;

        if (indexFindPos(keyname, pos))
        {
            // Key was found in the index, return the item pointed to by the key.  If not allowed
            // to update the existing item, return NULL.

            if (modifyfound)
            {
                DataElem* dat = indexGet(pos);
                if (dat)
                {
                    return &(dat->value);
                }
            }
            return NULL;
        }

        // Insert a new data element at the end (this will call default constructor).
        // Record the current length into the key that will be added to the index.

        int addloc = mData.length();

        DataElem* dat = mData.insert(addloc);

        // Set the key in the data object.

        dat->key.set(keyname);

        // Now, add the key to the index at the position determined by binary search earlier.

        int* index = mIndex.insert(pos);
        if (index)
        {
            *index = addloc;
        }

        // return a pointer to the value portion.

        return &(dat->value);
    }

    /**
     * Adds a new property
     *
     * @param  keyname Property key name
     *
     * @return         Pointer to the element or NULL if already exists
     */
    inline T* add(const char* keyname)
    {
        return addOrModify(keyname, false);
    }

    /**
     * Removes a property
     *
     * @param  keyname Property key name
     *
     * @return         Success
     */
    inline bool remove(const char* keyname)
    {
        bool ret = false;
        int pos;

        if (indexFindPos(keyname, pos))
        {
            int* datlocptr = mIndex.get(pos);
            if (datlocptr)
            {
                int dataloc = *datlocptr;

                // Remove the index entry

                mIndex.remove(pos);

                // Once the data from dataloc will be removed, all content
                // following it shifts.  Therefore, we must fixup the index entries.

                for (int i = 0; i < mIndex.length(); i++)
                {
                    int* p = mIndex.get(i);
                    if (*p >= dataloc)
                    {
                        (*p)--;
                    }
                }

                // Remove the data

                ret = mData.remove(dataloc);
            }
        }
        return ret;
    }

    /**
     * Returns a property element
     *
     * @param  keyname Property key name
     *
     * @return         Pointer to the element or NULL
     */
    inline T* get(const char* keyname)
    {
        int pos;
        if (indexFindPos(keyname, pos))
        {
            DataElem* dat = indexGet(pos);
            if (dat)
            {
                return &(dat->value);
            }
        }

        return NULL;
    }

    /**
     * Returns the element from a given position
     *
     * @param  pos Position
     *
     * @return     Pointer to the element or NULL
     */
    inline T* get(int pos)
    {
        DataElem* dat = mData.get(pos);
        if (dat)
        {
            return &(dat->value);
        }
        return NULL;
    }

    /**
     * Returns the property key at position
     *
     * @param  pos Position
     *
     * @return     Pointer to the property key name
     */
    inline const char* getKey(int pos)
    {
        DataElem* dat = mData.get(pos);
        if (dat)
        {
            return dat->key.get();
        }
        return NULL;
    }

    /**
     * Returns the length of the property array
     *
     * @return Number of elements
     */
    inline int length()
    {
        return mData.length();
    }

    /**
     * Returns an iterator to property elements in sorted order
     *
     * @param  iter Iterator
     *
     * @return      Success
     */
    bool forEachSort(Iter<T>& iter)
    {
        iter.mPos++;
        if (iter.mPos < mIndex.length())
        {
            DataElem* dat = indexGet(iter.mPos);
            if (dat)
            {
                iter.mObj = &(dat->value);
                iter.mKey = dat->key.get();
                return true;
            }
        }
        return false;
    }

    /**
     * Returns an iterator to property elements in original order
     *
     * @param  iter Iterator
     *
     * @return      Success
     */
    bool forEach(Iter<T>& iter)
    {
        iter.mPos++;
        if (iter.mPos < mData.length())
        {
            DataElem* dat = mData.get(iter.mPos);
            if (dat)
            {
                iter.mObj = &(dat->value);
                iter.mKey = dat->key.get();
                return true;
            }
        }
        return false;
    }

    /**
     * Clears the property array
     */
    inline void clear()
    {
        mData.clear();
        mIndex.clear();
        mClassName.clear();
        mSupp = NULL;
    }

/** \cond Internal */
    inline void setClassName(const char* name)
    {
        mClassName.set(name);
    }
    inline const char* getClassName()
    {
        return mClassName.get();
    }

    inline void setSuppData(void* supp)
    {
        mSupp = supp;
    }
    inline void* getSuppData()
    {
        return mSupp;
    }
    inline InterfaceImpl* setSupportImpl(InterfaceImpl* impl)
    {
        return mData.setSupportImpl(impl);
    }

    inline InterfaceImpl* getSupportImpl()
    {
        return mData.getSupportImpl();
    }

    void dbgDump()
    {
#ifdef _DEBUG
        dbglog("PropArray %p\n", this);
        dbglog("Index(%p): length=%d\n", &mIndex, mIndex.length());
        for (int i = 0; i < mIndex.length(); i++)
        {
            DataElem* dat = indexGet(i);
            if (dat == NULL)
            {
                dbglog("   %d -> %d NULL\n", i, *(mIndex.get(i)));
            }
            else
            {
                dbglog("   %d -> %d [%p %s]\n", i, *(mIndex.get(i)), &dat->value, dat->key.get());
            }
        }

        dbglog("Data(%p): length=%d\n", &mData, mData.length());
        for (int i = 0; i < mData.length(); i++)
        {
            dbglog("  %d ->  %p\n", i, (uchar*)(mData.get(i)) + sizeof(PropKeyStr));
        }
#endif
    }

/** \endcond Internal */

private:
    enum
    {
        INITSIZE = 2,   //TODO: consider making this 0
        FIXEDSTRSIZE = 24
    };

    typedef FixedStr<FIXEDSTRSIZE> PropKeyStr;
    struct DataElem
    {
        PropKeyStr key;
        T value;
    };

    ObjArray<DataElem> mData;
    ObjArray<int> mIndex;
    PropKeyStr mClassName;
    void* mSupp;

private:

    bool indexFindPos(const char* findelem, int& pos)
    {
        if (mIndex.length() == 0 || findelem == NULL)
        {
            pos = 0;
            return false;
        }

        int low = 0;
        int high = mIndex.length() - 1;
        while (low <= high)
        {
            int mid = (low + high) / 2;

            // Get the keyName which is stored at this index position
            // in the data array.

            DataElem* dat = indexGet(mid);

            int res = strcmp(dat->key.get(), findelem);

            if (res == 0)
            {
                pos = mid;
                return true;
            }
            else if (res < 0)
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }

        pos = low;
        return false;
    }

    inline DataElem* indexGet(int pos)
    {
        int* index = mIndex.get(pos);
        if (index)
        {
            return mData.get(*index);
        }
        return NULL;
    }

};


/**
 * StrArray is an array of character strings and maintains them in sorted order
 */
class StrArray : public ObjArray<std::string>
{
public:
    /**
     * Construct an empty StrArray.
     */
    StrArray() :
        ObjArray(StrArray::compare)
    {
    }

    /**
     * Adds a string to the array
     *
     * @param  keyelem String to add
     *
     * @return         Pointer to the stl::string element
     */
    inline std::string* add(const char* keyelem)
    {
        std::string s(keyelem);
        return ObjArray<std::string>::add(&s);
    }
    /**
     * Removes a string from the array
     *
     * @param  keyelem String to remove
     *
     * @return         Success
     */
    inline bool remove(const char* keyelem)
    {
        std::string s(keyelem);
        return ObjArray<std::string>::removeKey(&s);
    }
    /**
     * Finds a string in the array
     *
     * @param  elem String to find
     *
     * @return      Pointer to the stl::string element or NULL
     */
    inline std::string* find(const char* elem)
    {
        std::string s(elem);
        return ObjArray<std::string>::find(&s);
    }

public:
    /**
     * Compare \p e1 with \p e2 (by calling std::string.compare()).
     */
    static int compare(const void* e1, const void* e2)
    {
        const std::string* s1 = (std::string*)e1;
        const std::string* s2 = (std::string*)e2;

        return s1->compare(s2->c_str());
    }
};


/**
 * KeywordArray maintains a string keyword to integer relationship
 */
class KeywordArray
{
public:
    /**
     * A keyword-value pair.
     */
    struct Entry
    {
        const char* keyword; ///< The keyword into the array.
        uint value;          ///< The value of this item.
    };

    /**
     * constructor
     *
     * @param  arr     Pointer to an array which will be cloned
     * @param  arrsize Numer of elements in the array
     */
    KeywordArray(const Entry* arr, size_t arrsize) :
        mArr(arr),
        mArrSize(arrsize)
    {
        assert(arr && arrsize != 0);
    }

    /**
     * Converts a keyword into a int value
     *
     * @param  keyword Keword to lookup
     *
     * @return         Value or -1 if not found
     */
    uint toValue(const char* keyword)
    {
        assert(keyword);
        for (size_t i = 0; i < mArrSize; i++)
        {
            if (strcasecmp(keyword, mArr[i].keyword) == 0)
            {
                return mArr[i].value;
            }
        }
        return (uint)-1;
    }

    /**
     * Returns a keyword associated with the value
     *
     * @param  value Value to lookup
     *
     * @return       A pointer to the keyword
     */
    const char* toKeyword(uint value)
    {
        for (size_t i = 0; i < mArrSize; i++)
        {
            if (value == mArr[i].value)
            {
                return mArr[i].keyword;
            }
        }
        return NULL;
    }

private:
    const Entry* mArr;
    size_t mArrSize;
};

} // jvar

#endif // _ARR_H
