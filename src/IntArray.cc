//
//  Dynamic, self adjusting integer array
//
//  Assumptions:
//    - no bounds/range checking -> user responsibility
//    - the Register/Free methods called only by container class
//
#include "IntArray.hh"

//
// External integer array provided; responsibility of user 
// to manage memory
//
vlIntArray::Initialize(const int sz, const int ext)
{
  if ( this->Array != 0 ) delete [] this->Array;

  this->Size = ( sz > 0 ? sz : 1);
  if ( (this->Array = new int[sz]) == 0 ) return 0;
  this->Extend = ( ext > 0 ? ext : 1);
  this->MaxId = -1;

  return 1;
}

vlIntArray::vlIntArray(const int sz, const int ext)
{
  this->Size = ( sz > 0 ? sz : 1);
  this->Array = new int[sz];
  this->Extend = ( ext > 0 ? ext : 1);
  this->MaxId = -1;
}

vlIntArray::~vlIntArray()
{
  if ( this->Debug ) cerr << "Destructor\n";

  delete [] this->Array;
}

vlIntArray::vlIntArray(const vlIntArray& ia)
{
  int i;
  if ( this->Debug ) cerr << "Copy constructor\n";

  this->MaxId = ia.MaxId;
  this->Size = ia.Size;
  this->Extend = ia.Extend;

  this->Array = new int[this->Size];
  for (i=0; i<this->MaxId; i++)
    this->Array[i] = ia.Array[i];

}

vlIntArray& vlIntArray::operator=(vlIntArray& ia)
{
  int i;

  if ( this->Debug ) cerr << "Assignment\n";

  if ( this != &ia )
    {
    delete [] this->Array;

    this->MaxId = ia.MaxId;
    this->Size = ia.Size;
    this->Extend = ia.Extend;

    this->Array = new int[this->Size];
    for (i=0; i<=this->MaxId; i++)
      this->Array[i] = ia.Array[i];
    }
  return *this;
}

//
// Copy on write if used by more than one object
//
void vlIntArray::operator+=(vlIntArray& ia)
{
  int i, sz;

  if ( this->Debug ) cerr << "Add method\n";

  if ( this->Size <= (sz = this->MaxId + ia.MaxId + 2) ) this->Resize(sz);

  for (i=0; i<=ia.MaxId; i++)
    {
    this->Array[this->MaxId+1+i] = ia.Array[i];
    }
  this->MaxId += ia.MaxId + 1;

}

//
// Copy on write if used by more than one object
//
vlIntArray& vlIntArray::InsertValue(const int id, const int i)
{

  if ( this->Debug ) cerr << "insert value\n";

  if ( id >= this->Size ) this->Resize(id);

  this->Array[id] = i;
  if ( id > this->MaxId ) this->MaxId = id;

  return *this;
}

int vlIntArray::InsertNextValue(const int i)
{
  this->InsertValue (++this->MaxId,i);
  return this->MaxId;
}

void vlIntArray::Squeeze()
{
  this->Resize (this->MaxId+1);
}

int vlIntArray::GetSize()
{
  return this->Size;    
}

int vlIntArray::GetMaxId()
{
  return this->MaxId;
}

void vlIntArray::SetMaxId(int id)
{
  this->MaxId = (id < this->Size ? id : this->Size-1);
}

int *vlIntArray::GetArray()
{
  return this->Array;
}

void vlIntArray::Reset()
{
  this->MaxId = -1;
}
//
// Private function does "reallocate"
//
int *vlIntArray::Resize(const int sz)
{
  int i;
  int *newArray;
  int newSize;

  if ( this->Debug ) cerr << "Resize\n";

  if ( sz >= this->Size ) newSize = this->Size + 
    this->Extend*(((sz-this->Size)/this->Extend)+1);
  else newSize = sz;

  if ( (newArray = new int[newSize]) == 0 )
    {
    cerr << "Cannot allocate memory\n";
    return 0;
    }

  for (i=0; i<sz && i<this->Size; i++)
      newArray[i] = this->Array[i];

  this->Size = newSize;
  delete [] this->Array;
  this->Array = newArray;

  return this->Array;
}
