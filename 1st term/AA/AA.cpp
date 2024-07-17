#include <cstring>
#include <iostream>

struct Node {
  Node* next = nullptr;
  char* string;
  int size = 1;
};

char* InputString() {
  char* result;
  char current_char;
  int length = 0;
  int size = 6;

  result = (char*)realloc(NULL, sizeof(*result) * size);

  while ((current_char = std::cin.get()) != EOF && current_char != ' ' &&
         current_char != '\n') {
    result[length++] = current_char;

    if (length == size) {
      size *= 2;
      result = (char*)realloc(result, sizeof(*result) * size);
    }
  }

  result[length] = '\0';

  return (char*)realloc(result, sizeof(*result) * length);
}

void Push(Node*& top_elem) {
  if (top_elem == nullptr) {
    top_elem = new Node;
    top_elem->string = InputString();
  } else {
    Node* new_top_elem;
    new_top_elem = new Node;
    new_top_elem->next = top_elem;
    new_top_elem->size = top_elem->size + 1;
    top_elem = new_top_elem;
    top_elem->string = InputString();
  }

  std::cout << "ok\n";
}

void Pop(Node*& top_elem) {
  if (top_elem == nullptr) {
    std::cout << "error\n";
  } else {
    std::cout << top_elem->string << "\n";
    Node* new_top_elem;
    new_top_elem = top_elem->next;
    delete top_elem;
    top_elem = new_top_elem;
  }
}

void Back(Node*& top_elem) {
  if (top_elem == nullptr) {
    std::cout << "error\n";
  } else {
    std::cout << top_elem->string << "\n";
  }
}

void Clear(Node*& top_elem) {
  while (top_elem != nullptr) {
    Node* new_top_elem;
    new_top_elem = top_elem->next;
    delete top_elem;
    top_elem = new_top_elem;
  }

  std::cout << "ok\n";
}

void Exit(Node*& top_elem) {
  while (top_elem != nullptr) {
    Node* new_top_elem;
    new_top_elem = top_elem->next;
    delete top_elem;
    top_elem = new_top_elem;
  }

  std::cout << "bye\n";
}

int main() {
  Node* top_elem = nullptr;
  char* command;

  while (true) {
    command = InputString();

    if (command[0] == 'p' && command[1] == 'u') {
      Push(top_elem);
    } else if (command[0] == 'p') {
      Pop(top_elem);
    } else if (command[0] == 'b') {
      Back(top_elem);
    } else if (command[0] == 's') {
      if (top_elem != nullptr) {
        std::cout << top_elem->size << "\n";
      } else {
        std::cout << 0 << "\n";
      }
    } else if (command[0] == 'c') {
      Clear(top_elem);
    } else if (command[0] == 'e') {
      Exit(top_elem);
      free(command);
      free(top_elem);
      break;
    }

    free(command);
  }
}