#pragma once

template <typename T>
class DynamicArray {
public:
    DynamicArray(const unsigned int capacity);
    ~DynamicArray();
    T& Push(const T& data);
    T Pop();
    T& At(const unsigned int index);
    void SetAt(const T& data, const unsigned int index);
    const unsigned int Size();
    const unsigned int Capacity();
    T* GetArrPointer();
    void SetArrPointer(T* pArr, unsigned int nrOfElements);
    void Clear();
    void Delete();
    void Sort();
private:
    T* mArr;
    unsigned int mCapacity;
    unsigned int mNrOfElements;
    void QuickSort(T* arr, int left, int right);
};

template <typename T>
inline DynamicArray<T>::DynamicArray(const unsigned int capacity)
{
    mArr = new T[capacity];
    mCapacity = capacity;
    mNrOfElements = 0;
}

template <typename T>
inline DynamicArray<T>::~DynamicArray()
{

}

template <typename T>
inline T& DynamicArray<T>::Push(const T& data)
{
    assert(mNrOfElements < mCapacity);
    mArr[mNrOfElements] = data;
    return mArr[mNrOfElements++];
}

template <typename T>
inline T DynamicArray<T>::Pop()
{
    if (mNrOfElements > 0)
        return mArr[--mNrOfElements];
    return T();
}

template <typename T>
inline T& DynamicArray<T>::At(const unsigned int index)
{
    assert(index < mNrOfElements);
    return mArr[index];
}

template <typename T>
inline void DynamicArray<T>::SetAt(const T& data, const unsigned int index)
{
    assert(index < mNrOfElements);
    mArr[index] = data;
}

template <typename T>
inline const unsigned int DynamicArray<T>::Size()
{
    return mNrOfElements;
}

template <typename T>
inline const unsigned int DynamicArray<T>::Capacity()
{
    return mCapacity;
}

template <typename T>
inline T* DynamicArray<T>::GetArrPointer()
{
    return mArr;
}

template <typename T>
inline void DynamicArray<T>::SetArrPointer(T* pArr, unsigned int nrOfElements)
{
    Clear();
    mArr = pArr;
    mNrOfElements = nrOfElements;
}

template <typename T>
inline void DynamicArray<T>::Clear()
{
    mNrOfElements = 0;
}

template <typename T>
inline void DynamicArray<T>::Delete()
{
    mNrOfElements = 0;
    delete[] mArr;
    mArr = nullptr;
}

template <typename T>
inline void DynamicArray<T>::Sort()
{
    assert(mNrOfElements != 0);
    QuickSort(mArr, 0, mNrOfElements);
}

template <typename T>
inline void DynamicArray<T>::QuickSort(T* arr, int left, int right)
{
    /* http://www.algolist.net/Algorithms/Sorting/Quicksort */
    int i = left, j = right;
    T tmp;
    T pivot = arr[(left + right) / 2];

    /* partition */
    while (i <= j) {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    };

    /* recursion */
    if (left < j)
        QuickSort(arr, left, j);
    if (i < right)
        QuickSort(arr, i, right);
}
