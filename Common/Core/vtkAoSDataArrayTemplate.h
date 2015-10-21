/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAoSDataArrayTemplate.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAoSDataArrayTemplate - vtkGenericDataArray specialization that
// stores data array in the traditional VTK memory layout where a 3 component is
// stored in contiguous memory as \c A1A2A2B1B2B3C1C2C3 ... where A,B,C,... are
// tuples.
// .SECTION Description
// This replaces vtkDataArrayTemplate.

#ifndef vtkAoSDataArrayTemplate_h
#define vtkAoSDataArrayTemplate_h

#include "vtkGenericDataArray.h"
#include "vtkBuffer.h"

template <class ValueTypeT>
class vtkAoSDataArrayTemplate :
  public vtkTypeTemplate<
          vtkAoSDataArrayTemplate<ValueTypeT>,
          vtkGenericDataArray<vtkAoSDataArrayTemplate<ValueTypeT>, ValueTypeT>
         >
{
  typedef vtkGenericDataArray<vtkAoSDataArrayTemplate<ValueTypeT>, ValueTypeT >
          GenericDataArrayType;
public:
  typedef GenericDataArrayType Superclass;
  typedef vtkAoSDataArrayTemplate<ValueTypeT> SelfType;
  typedef typename Superclass::ValueType ValueType;
  typedef typename Superclass::ReferenceType ReferenceType;

  // Description:
  // Legacy support for array-of-structs value iteration.
  // TODO Deprecate?
  typedef ValueType* Iterator;
  Iterator Begin() { return Iterator(this->GetVoidPointer(0)); }
  Iterator End() { return Iterator(this->GetVoidPointer(this->MaxId + 1)); }

  static vtkAoSDataArrayTemplate* New();

  // **************************************************************************
  // Methods that are needed to be implemented by every vtkGenericDataArray
  // subclass.
  // **************************************************************************
  inline const ReferenceType GetValue(vtkIdType valueIdx) const
    {
    return this->Buffer.GetBuffer()[valueIdx];
    }
  inline void GetTupleValue(vtkIdType tupleIdx, ValueType* tuple) const
    {
    const vtkIdType valueIdx = tupleIdx * this->NumberOfComponents;
    std::copy(this->Buffer.GetBuffer() + valueIdx,
              this->Buffer.GetBuffer() + valueIdx + this->NumberOfComponents,
              tuple);
    }
  inline const ReferenceType GetComponentValue(vtkIdType index, int comp) const
    {
    return this->Buffer.GetBuffer()[this->NumberOfComponents*index + comp];
    }
  inline void SetValue(vtkIdType valueIdx, ValueType value)
    {
    this->Buffer.GetBuffer()[valueIdx] = value;
    }
  inline void SetTupleValue(vtkIdType tupleIdx, const ValueType* tuple)
    {
    const vtkIdType valueIdx = tupleIdx * this->NumberOfComponents;
    std::copy(tuple, tuple + this->NumberOfComponents,
              this->Buffer.GetBuffer() + valueIdx);
    }
  inline void SetComponentValue(vtkIdType tupleIdx, int comp, ValueType value)
    {
    const vtkIdType valueIdx = tupleIdx * this->NumberOfComponents + comp;
    this->SetValue(valueIdx, value);
    }

  // **************************************************************************
  // Description:
  // Get the address of a particular data index. Make sure data is allocated
  // for the number of items requested. Set MaxId according to the number of
  // data values requested.
  ValueType* WritePointer(vtkIdType id, vtkIdType number);
  virtual void* WriteVoidPointer(vtkIdType id, vtkIdType number)
    { return this->WritePointer(id, number); }

  // Description:
  // Get the address of a particular data index. Performs no checks
  // to verify that the memory has been allocated etc.
  // If the data is simply being iterated over, consider using
  // vtkDataArrayIteratorMacro for safety and efficiency, rather than using this
  // member directly.
  ValueType* GetPointer(vtkIdType id) { return this->Buffer.GetBuffer() + id; }
  virtual void* GetVoidPointer(vtkIdType id) { return this->GetPointer(id); }

  enum DeleteMethod
    {
    VTK_DATA_ARRAY_FREE=vtkBuffer<ValueType>::VTK_DATA_ARRAY_FREE,
    VTK_DATA_ARRAY_DELETE=vtkBuffer<ValueType>::VTK_DATA_ARRAY_DELETE
    };

  // Description:
  // This method lets the user specify data to be held by the array.  The
  // array argument is a pointer to the data.  size is the size of the
  // array supplied by the user.  Set save to 1 to keep the class from
  // deleting the array when it cleans up or reallocates memory.  The class
  // uses the actual array provided; it does not copy the data from the
  // suppled array. If specified, the delete method determines how the data
  // array will be deallocated. If the delete method is
  // VTK_DATA_ARRAY_FREE, free() will be used. If the delete method is
  // DELETE, delete[] will be used. The default is FREE.
  void SetArray(ValueType* array, vtkIdType size, int save, int deleteMethod);
  void SetArray(ValueType* array, vtkIdType size, int save)
    { this->SetArray(array, size, save, VTK_DATA_ARRAY_FREE); }
  virtual void SetVoidArray(void* array, vtkIdType size, int save)
    { this->SetArray(static_cast<ValueType*>(array), size, save); }
  virtual void SetVoidArray(void* array, vtkIdType size, int save,
                            int deleteMethod)
    {
    this->SetArray(static_cast<ValueType*>(array), size, save, deleteMethod);
    }

  // Description:
  // Tell the array explicitly that a single data element has
  // changed. Like DataChanged(), then is only necessary when you
  // modify the array contents without using the array's API.
  // @note This is a legacy method from vtkDataArrayTemplate, and is only
  // implemented for array-of-struct arrays. It currently just calls
  // DataChanged() and does nothing clever.
  virtual void DataElementChanged(vtkIdType)
  {
    this->DataChanged();
  }

  virtual vtkArrayIterator *NewIterator();

  // Description:
  // Perform a fast, safe cast from a vtkAbstractArray to a
  // vtkAoSDataArrayTemplate.
  // This method checks if source->GetArrayType() returns DataArray
  // or a more derived type, and performs a static_cast to return
  // source as a vtkDataArray pointer. Otherwise, NULL is returned.
  vtkAoSDataArrayTemplate<ValueType>* FastDownCast(vtkAbstractArray *source)
  {
    switch (source->GetArrayType())
      {
      case vtkAbstractArray::AoSDataArrayTemplate:
        if (source->GetDataType() == vtkTypeTraits<ValueType>::VTK_TYPE_ID)
          {
          return static_cast<vtkAoSDataArrayTemplate<ValueType>*>(source);
          }
        break;
      }
    return NULL;
  }

  // Description:
  // Method for type-checking in FastDownCast implementations.
  virtual int GetArrayType() { return vtkAbstractArray::AoSDataArrayTemplate; }

//BTX
protected:
  vtkAoSDataArrayTemplate();
  ~vtkAoSDataArrayTemplate();

  // **************************************************************************
  // Methods that are needed to be implemented by every vtkGenericDataArray
  // subclass.
  // **************************************************************************
  // Implement the memory management interface.
  bool AllocateTuples(vtkIdType numTuples);
  bool ReallocateTuples(vtkIdType numTuples);
  // **************************************************************************

  vtkBuffer<ValueType> Buffer;
  ValueType ValueRange[2]; // XXX
  bool SaveUserArray;
  int DeleteMethod;

private:
  vtkAoSDataArrayTemplate(const vtkAoSDataArrayTemplate&); // Not implemented.
  void operator=(const vtkAoSDataArrayTemplate&); // Not implemented.
  friend class vtkGenericDataArray<vtkAoSDataArrayTemplate<ValueTypeT>,
                                   ValueTypeT>;
//ETX
};

#include "vtkAoSDataArrayTemplate.txx"

#endif // header guard

// TODO clean this up, more or less copy/pasted from vtkDataArrayTemplate.h:

#if !defined(VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION)
# define VTK_AOS_DATA_ARRAY_TEMPLATE_INSTANTIATE(T) \
   template class VTKCOMMONCORE_EXPORT vtkAoSDataArrayTemplate< T >
#else
// TODO Not sure what this does, need to dig some:
//# include "vtkDataArrayTemplateImplicit.txx"
//# define VTK_AOS_DATA_ARRAY_TEMPLATE_INSTANTIATE(T)
#endif

// This portion must be OUTSIDE the include blockers.  Each
// vtkDataArray subclass uses this to give its instantiation of this
// template a DLL interface.
#if defined(VTK_DATA_ARRAY_TEMPLATE_TYPE) && !defined(VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION)
# if defined(VTK_BUILD_SHARED_LIBS) && defined(_MSC_VER)
#  pragma warning (push)
#  pragma warning (disable: 4091) // warning C4091: 'extern ' :
   // ignored on left of 'int' when no variable is declared
#  pragma warning (disable: 4231) // Compiler-specific extension warning.

   // We need to disable warning 4910 and do an extern dllexport
   // anyway.  When deriving vtkCharArray and other types from an
   // instantiation of this template the compiler does an explicit
   // instantiation of the base class.  From outside the vtkCommon
   // library we block this using an extern dllimport instantiation.
   // For classes inside vtkCommon we should be able to just do an
   // extern instantiation, but VS 2008 complains about missing
   // definitions.  We cannot do an extern dllimport inside vtkCommon
   // since the symbols are local to the dll.  An extern dllexport
   // seems to be the only way to convince VS 2008 to do the right
   // thing, so we just disable the warning.
#  pragma warning (disable: 4910) // extern and dllexport incompatible

   // Use an "extern explicit instantiation" to give the class a DLL
   // interface.  This is a compiler-specific extension.
   extern VTK_AOS_DATA_ARRAY_TEMPLATE_INSTANTIATE(VTK_DATA_ARRAY_TEMPLATE_TYPE);
#  pragma warning (pop)
# endif
# undef VTK_DATA_ARRAY_TEMPLATE_TYPE
#endif

// This macro is used by the subclasses to create dummy
// declarations for these functions such that the wrapper
// can see them. The wrappers ignore vtkDataArrayTemplate.
#define vtkCreateWrappedArrayInterface(T) \
  int GetDataType(); \
  void GetTupleValue(vtkIdType i, T* tuple); \
  void SetTupleValue(vtkIdType i, const T* tuple); \
  void InsertTupleValue(vtkIdType i, const T* tuple); \
  vtkIdType InsertNextTupleValue(const T* tuple); \
  T GetValue(vtkIdType id); \
  void SetValue(vtkIdType id, T value); \
  void SetNumberOfValues(vtkIdType number); \
  void InsertValue(vtkIdType id, T f); \
  vtkIdType InsertNextValue(T f); \
  T *GetValueRange(int comp); \
  T *GetValueRange(); \
  T* WritePointer(vtkIdType id, vtkIdType number); \
  T* GetPointer(vtkIdType id)/*; \

  * These methods are not wrapped to avoid wrappers exposing these
  * easy-to-get-wrong methods because passing in the wrong value for 'save' is
  * guaranteed to cause a memory issue down the line. Either the wrappers
  * didn't use malloc to allocate the memory or the memory isn't actually
  * persisted because a temporary array is used that doesn't persist like this
  * method expects.

  void SetArray(T* array, vtkIdType size, int save); \
  void SetArray(T* array, vtkIdType size, int save, int deleteMethod) */
