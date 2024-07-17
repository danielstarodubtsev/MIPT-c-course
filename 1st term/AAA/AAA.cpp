#include <cstring>
#include <iostream>

void NextIndexesArray(int* arr_indexes, int* pref, int argc) {
  int current_index = argc - 2;
  while (current_index >= 0 && 
        (arr_indexes[current_index] == pref[current_index + 1] - pref[current_index] - 1)) {
    arr_indexes[current_index] = 0;
    --current_index;
  }
  if (current_index == -1) { return; }
  ++arr_indexes[current_index];
}

bool CheckIndexes(int* arr_indexes, int argc) {
  for (int i = 0; i < argc - 1; ++i) {
    for (int j = 0; j < argc - 1; ++j) {
      if (i != j && arr_indexes[i] == arr_indexes[j]) {
        return false;
      }
    }
  }
  return true;
}

int GetProduct(int* arr_indexes, int* arr, int* pref, int argc) {
  if (!CheckIndexes(arr_indexes, argc)) {
    return 0;
  }
  long long result = 1;
  for (int i = 0; i < argc - 1; ++i) {
    result *= arr[pref[i] + arr_indexes[i]];
  }
  return result;
}

int main(int argc, char* argv[]) {
  int sum = 0;
  int product = 1;
  for (int i = 1; i < argc; ++i) {
    sum += atoi(argv[i]);
    product *= atoi(argv[i]);
  }

  int* arr = new int[sum];
  for (int i = 0; i < sum; ++i) {
    std::cin >> arr[i];
  }

  int* pref = new int[argc];
  pref[0] = 0;
  for (int i = 1; i < argc; ++i) {
    pref[i] = pref[i - 1] + atoi(argv[i]);
  }

  int* arr_indexes = new int[argc - 1];
  memset(arr_indexes, 0, (argc - 1) * sizeof(int));

  long long total_sum = 0;
  for (int i = 0; i < product; ++i) {
    total_sum += GetProduct(arr_indexes, arr, pref, argc);
    NextIndexesArray(arr_indexes, pref, argc);
  }

  delete [] arr;
  delete [] pref;
  delete [] arr_indexes;

  std::cout << total_sum;
}