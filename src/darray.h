
/*
    Basic Dynamic Array

    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
        https://github.com/emelianov/modbus-esp8266
        This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
template <typename ElementType, int INITIAL_SIZE, int CAPACITY_INCREMENT> class DArray {
public:
  typedef bool (*Compare)(ElementType);
  ElementType* data = nullptr;
  size_t resSize = 0;
  size_t last = 0;
  bool isEmpty = true;
  DArray(size_t initialCapacity = INITIAL_SIZE) {
    data = (ElementType*)malloc(initialCapacity * sizeof(ElementType));
    if (data) resSize = initialCapacity;
  }
  size_t push_back(const ElementType& value) {
    if (!data) {
      data = (ElementType*)malloc(resSize * sizeof(ElementType));
      if (!data) {
        return 1;
      }
    }
    if (last >= resSize - 1) {
      if (CAPACITY_INCREMENT == 0) return last + 1;
      void* resizedData = realloc(data, (resSize + CAPACITY_INCREMENT) * sizeof(ElementType));
      if (!resizedData) return last + 1;
      resSize += CAPACITY_INCREMENT;
      data = (ElementType*)resizedData;
    }
    if (!isEmpty) {
      last++;
    } else {
      isEmpty = false;
    }
    data[last] = value;
    return last;
  }
  size_t size() {
    if (isEmpty) {
      return 0;
    }
    return last + 1;
  }
  template <class UnaryPredicate> size_t find(UnaryPredicate predicate, size_t startIndex = 0) {
    if (isEmpty) {
      return 1;
    }
    for (; startIndex <= last; startIndex++) {
      if (predicate(data[startIndex])) {
        break;
      }
    }
    return startIndex;
  }

  void remove(size_t index) {
    if (isEmpty) {
      return;
    }
    if (index > last) {
      return;
    }
    if (last == 0) {
      isEmpty = true;
      return;
    }
    if (index < last) {
      memcpy(&data[index], &data[index + 1], (last - index) * sizeof(ElementType));
    }
    last--;
  }
  ElementType operator[](int index) { return data[index]; }
  ElementType* entry(size_t index) {
    if (index > last) {
      return nullptr;
    }
    return &data[index];
  }
};
