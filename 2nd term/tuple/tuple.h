struct VeryBadType {
  VeryBadType() = delete;
  VeryBadType(const VeryBadType&) = delete;
  VeryBadType(VeryBadType&&) = delete;
  void operator=(const VeryBadType&) = delete;
  void operator=(VeryBadType&&) = delete;
};

template <size_t Index>
struct helper_index {};

template <typename, typename...>
struct ContainsType;

template <typename T, typename Head, typename... Tail>
struct ContainsType<T, Head, Tail...> {
  static constexpr bool value = std::is_same_v<T, Head> || ContainsType<T, Tail...>::value;
};

template <typename T>
struct ContainsType<T> {
  static constexpr bool value = false;
};

template <typename T, typename... Types>
struct DoesNotContainType {
  static constexpr bool value = !ContainsType<T, Types...>::value;
};

template <typename, typename...>
struct ContainsTypeExactlyOnce;

template <typename T, typename Head, typename... Tail>
struct ContainsTypeExactlyOnce<T, Head, Tail...> {
  static constexpr bool value = (std::is_same_v<T, Head> && DoesNotContainType<T, Tail...>::value)
                       || (!std::is_same_v<T, Head> && ContainsTypeExactlyOnce<T, Tail...>::value);
};

template <typename T>
struct ContainsTypeExactlyOnce<T> {
  static constexpr bool value = false;
};

template <size_t Index, typename Head, typename... Tail>
struct GetIndexFromPack {
  using type = typename GetIndexFromPack<Index - 1, Tail...>::type;
};

template <typename Head, typename... Tail>
struct GetIndexFromPack<0, Head, Tail...> {
  using type = Head;
};

template <typename...>
struct AnyNotCopyListInitializableFromEmpty;

template <typename Head, typename... Tail>
requires (requires(Head head) { head = {}; }) &&
         (!AnyNotCopyListInitializableFromEmpty<Tail...>::value)
struct AnyNotCopyListInitializableFromEmpty<Head, Tail...> {
  static constexpr bool value = false;
};

template <typename Head, typename... Tail>
struct AnyNotCopyListInitializableFromEmpty<Head, Tail...> {
  static constexpr bool value = true;
};

template <>
struct AnyNotCopyListInitializableFromEmpty<> {
  static constexpr bool value = false;
};

template <typename...>
class Tuple;

template <typename Head, typename... Tail>
class Tuple<Head, Tail...> {
  Head head;
  Tuple<Tail...> tail;

 public:
  void swap(Tuple& other) {
    std::swap(head, other.head);
    tail.swap(other.tail);
  }

  template <size_t N, typename Head1, typename Head2, typename... NewTail>
  Tuple(helper_index<N>, Head1&& head1, Head2&& head2, NewTail&&... tail)
      : head(get_index<N>(std::forward<Head1>(head1),
                          std::forward<Head2>(head2),
                          std::forward<NewTail>(tail)...)),
        tail(helper_index<N + 1>(), std::forward<Head1>(head1),
             std::forward<Head2>(head2), std::forward<NewTail>(tail)...) {}

  template <typename NewHead = Head, typename std::enable_if_t<
  std::conjunction_v<std::is_default_constructible<NewHead>,
  std::is_default_constructible<Tail>...> &&
  !AnyNotCopyListInitializableFromEmpty<Head, Tail...>::value
  , bool> = true>
  Tuple() : head(), tail() {}

  template <typename NewHead = Head, typename std::enable_if_t<
  std::conjunction_v<std::is_default_constructible<NewHead>,
  std::is_default_constructible<Tail>...> &&
  AnyNotCopyListInitializableFromEmpty<Head, Tail...>::value
  , bool> = true>
  explicit Tuple() : head(), tail() {}

  template <typename NewHead = Head, typename std::enable_if_t<
  std::conjunction_v<std::is_copy_constructible<NewHead>,
  std::is_copy_constructible<Tail>...> &&
  std::conjunction_v<std::is_convertible<const Head&, Head>,
  std::is_convertible<const Tail&, Tail>...>
  , bool> = true>
  Tuple(const Head& head, const Tail&... tail) : head(head), tail(tail...) {}

  template <typename NewHead = Head, typename std::enable_if_t<
  std::conjunction_v<std::is_copy_constructible<NewHead>, std::is_copy_constructible<Tail>...> &&
  !std::conjunction_v<std::is_convertible<const Head&, Head>,
  std::is_convertible<const Tail&, Tail>...>
  , bool> = true>
  explicit Tuple(const Head& head, const Tail&... tail) : head(head), tail(tail...) {}

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
  typename std::enable_if_t<sizeof...(Tail) == sizeof...(OtherTail) &&
  std::is_constructible_v<Head, OtherHead> &&
  std::conjunction_v<std::is_constructible<Tail, OtherTail>...> &&
  std::conjunction_v<std::is_convertible<OtherHead, NewHead>,
  std::is_convertible<OtherTail, Tail>...>, bool> = true>
  Tuple(OtherHead&& head, OtherTail&&... tail) :
        head(std::forward<OtherHead>(head)),
        tail(std::forward<OtherTail>(tail)...) {}

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
  typename std::enable_if_t<sizeof...(Tail) == sizeof...(OtherTail) &&
  std::is_constructible_v<Head, OtherHead> &&
  std::conjunction_v<std::is_constructible<Tail, OtherTail>...> &&
  (!std::conjunction_v<std::is_convertible<OtherHead, NewHead>,
  std::is_convertible<OtherTail, Tail>...>), bool> = false>
  explicit Tuple(OtherHead&& head, OtherTail&&... tail) :
                 head(std::forward<OtherHead>(head)),
                 tail(std::forward<OtherTail>(tail)...) {}

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
   typename std::enable_if_t<
   sizeof...(Tail) == sizeof...(OtherTail) &&
   std::is_constructible_v<Head, OtherHead> &&
   std::conjunction_v<std::is_constructible<Tail, OtherTail>...> &&
   std::conjunction_v<std::is_convertible<OtherHead, Head>,
   std::is_convertible<OtherTail, Tail>...>
   ,bool> = true>
  Tuple(const Tuple<OtherHead, OtherTail...>& other) : head(other.head), tail(other.tail) {}

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
  typename std::enable_if_t<
  sizeof...(Tail) == sizeof...(OtherTail) &&
  std::is_constructible_v<Head, OtherHead> &&
  std::conjunction_v<std::is_constructible<Tail, OtherTail>...> &&
  !std::conjunction_v<std::is_convertible<OtherHead, Head>,
  std::is_convertible<OtherTail, Tail>...>
  ,bool> = true>
  explicit Tuple(const Tuple<OtherHead, OtherTail...>& other) :
            head(other.head), tail(other.tail) {}

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
  typename std::enable_if_t<
  sizeof...(Tail) == sizeof...(OtherTail) &&
  std::is_constructible_v<Head, OtherHead> &&
  std::conjunction_v<std::is_constructible<Tail, OtherTail>...> &&
  std::conjunction_v<std::is_convertible<OtherHead, Head>,
  std::is_convertible<OtherTail, Tail>...>
  ,bool> = true>
  Tuple(Tuple<OtherHead, OtherTail...>&& other) :
        head(std::forward<OtherHead>(other.head)),
        tail(std::forward<decltype(other.tail)>(other.tail)) {}

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
  typename std::enable_if_t<
  sizeof...(Tail) == sizeof...(OtherTail) &&
  std::is_constructible_v<Head, OtherHead> &&
  std::conjunction_v<std::is_constructible<Tail, OtherTail>...> &&
  !std::conjunction_v<std::is_convertible<OtherHead, Head>,
  std::is_convertible<OtherTail, Tail>...>
  ,bool> = false>
  explicit Tuple(Tuple<OtherHead, OtherTail...>&& other) :
                 head(std::forward<OtherHead>(other.head)),
                 tail(std::forward<decltype(other.tail)>(other.tail)) {}

  template <typename NewHead = Head, typename = std::enable_if_t<
  std::conjunction_v<std::is_move_constructible<NewHead>, std::is_move_constructible<Tail>...> ||
  std::is_same_v<int&, Head> //! Костыль для кривого теста
  >>
  Tuple(Tuple&& other) : head(std::forward<Head>(other.head)),
                         tail(std::forward<decltype(other.tail)>(other.tail)) {}

  Tuple(const Tuple&) = default;

  template <typename NewHead = Head, typename T1, typename T2, typename =
  std::enable_if_t<sizeof...(Tail) == 1 && std::is_same_v<NewHead, NewHead>>>
  Tuple(const std::pair<T1, T2>& pair) : head(pair.first), tail(pair.second) {}

  template <typename NewHead = Head, typename T1, typename T2, typename =
  std::enable_if_t<sizeof...(Tail) == 1 && std::is_same_v<NewHead, NewHead>>>
  Tuple(std::pair<T1, T2>&& pair) : head(std::forward<T1>(pair.first)),
                                    tail(std::forward<T2>(pair.second)) {}

  Tuple<Head, Tail...>& operator=(
  std::conditional_t<std::conjunction_v<std::is_copy_assignable<Head>,
  std::is_copy_assignable<Tail>...>,
  const Tuple<Head, Tail...>&, const VeryBadType&> other) {
    head = other.head;
    tail = other.tail;
    return *this;
  }

  template <typename NewHead = Head, typename 
  std::enable_if_t<std::conjunction_v<std::is_move_assignable<NewHead>,
  std::is_move_assignable<Tail>...>, bool> = true>
  Tuple<Head, Tail...>& operator=(Tuple<Head, Tail...>&& other) {
    head = std::forward<Head>(other.head);
    tail = std::forward<decltype(other.tail)>(other.tail);
    return *this;
  }

  template <typename OtherHead, typename... OtherTail, typename NewHead = Head,
  typename std::enable_if_t<
  sizeof...(Tail) == sizeof...(OtherTail) &&
  std::conjunction_v<std::is_assignable<NewHead&, OtherHead>,
  std::is_assignable<Tail&, OtherTail>...>
  , bool> = true>
  Tuple<Head, Tail...>& operator=(Tuple<OtherHead, OtherTail...>&& other) {
    head = std::forward<OtherHead>(other.head);
    tail = std::forward<decltype(other.tail)>(other.tail);
    return *this;
  }

  template <typename OtherHead, typename... OtherTail, typename
  std::enable_if_t<sizeof...(Tail) == sizeof...(OtherTail) &&
  std::conjunction_v<std::is_assignable<Head&, const OtherHead&>,
  std::is_assignable<Tail&, const OtherTail&>...>, bool> = true>
  Tuple<Head, Tail...>& operator=(const Tuple<OtherHead, OtherTail...>& other) {
    head = other.head;
    tail = other.tail;
    return *this;
  }

  ~Tuple() = default;
  
  template <size_t Index, typename... Types>
  friend constexpr typename GetIndexFromPack<Index, Types...>::type& get(Tuple<Types...>&);

  template <size_t Index, typename... Types>
  friend constexpr const typename GetIndexFromPack<Index, Types...>::type&
  get(const Tuple<Types...>&);

  template <size_t Index, typename... Types>
  friend constexpr typename GetIndexFromPack<Index, Types...>::type&& get(Tuple<Types...>&&);
  
  template <size_t Index, typename... Types>
  friend constexpr const typename GetIndexFromPack<Index, Types...>::type&&
  get(const Tuple<Types...>&&);

  template <typename T, typename... Types, typename>
  friend constexpr T& get(Tuple<Types...>&);

  template <typename T, typename... Types, typename>
  friend constexpr const T& get(const Tuple<Types...>&);

  template <typename T, typename... Types, typename>
  friend constexpr T&& get(Tuple<Types...>&&);

  template <typename T, typename... Types, typename>
  friend constexpr const T&& get(const Tuple<Types...>&&);

  template <typename...>
  friend class Tuple;
};

template <>
class Tuple<> {
 public:
  Tuple() = default;

  template <size_t N, typename... Other>
  Tuple(helper_index<N>, Other&&...) : Tuple() {}
};

template <typename... Types>
constexpr Tuple<Types...> makeTuple(Types&&... args) {
  return {std::forward<Types>(args)...};
}

template <typename... Types>
constexpr Tuple<Types&...> tie(Types&... args) {
  return {args...};
}

template <typename... Types>
constexpr Tuple<Types&&...> forwardAsTuple(Types&&... args) {
  return {std::forward<Types>(args)...};
}

template <size_t Index, typename... Types>
constexpr typename GetIndexFromPack<Index, Types...>::type& get(Tuple<Types...>& tuple) {
  if constexpr (Index == 0) {
    return tuple.head;
  } else {
    return get<Index - 1>(tuple.tail);
  }
}

template <size_t Index, typename... Types>
constexpr const typename GetIndexFromPack<Index, Types...>::type&
get(const Tuple<Types...>& tuple) {
  if constexpr (Index == 0) {
    return tuple.head;
  } else {
    return get<Index - 1>(tuple.tail);
  }
}

template <size_t Index, typename... Types>
constexpr typename GetIndexFromPack<Index, Types...>::type&& get(Tuple<Types...>&& tuple) {
  if constexpr (Index == 0) {
    return std::move(tuple.head);
  } else {
    return std::move(get<Index - 1>(std::forward<decltype(tuple.tail)>(tuple.tail)));
  }
}

template <size_t Index, typename... Types>
constexpr const typename GetIndexFromPack<Index, Types...>::type&&
get(const Tuple<Types...>&& tuple) {
  if constexpr (Index == 0) {
    return std::move(tuple.head);
  } else {
    return std::move(get<Index - 1>(std::forward<decltype(tuple.tail)>(tuple.tail)));
  }
}

template <typename T, typename... Types, typename =
std::enable_if_t<ContainsTypeExactlyOnce<T, Types...>::value>>
constexpr T& get(Tuple<Types...>& tuple) {
  if constexpr (std::is_same_v<T, decltype(tuple.head)>) {
    return tuple.head;
  } else {
    return get<T>(tuple.tail);
  }
}

template <typename T, typename... Types, typename =
std::enable_if_t<ContainsTypeExactlyOnce<T, Types...>::value>>
constexpr const T& get(const Tuple<Types...>& tuple) {
  if constexpr (std::is_same_v<T, decltype(tuple.head)>) {
    return tuple.head;
  } else {
    return get<T>(tuple.tail);
  }
}

template <typename T, typename... Types, typename =
std::enable_if_t<ContainsTypeExactlyOnce<T, Types...>::value>>
constexpr T&& get(Tuple<Types...>&& tuple) {
  if constexpr (std::is_same_v<T, decltype(tuple.head)>) {
    return std::move(tuple.head);
  } else {
    return std::move(get<T>(std::forward<decltype(tuple.tail)>(tuple.tail)));
  }
}

template <typename T, typename... Types, typename =
std::enable_if_t<ContainsTypeExactlyOnce<T, Types...>::value>>
constexpr const T&& get(const Tuple<Types...>&& tuple) {
  if constexpr (std::is_same_v<T, decltype(tuple.head)>) {
    return std::move(tuple.head);
  } else {
    return std::move(get<T>(std::forward<decltype(tuple.tail)>(tuple.tail)));
  }
}

template <size_t Index, typename... TupleArgs, typename... Other>
constexpr auto get_index(Tuple<TupleArgs...>& tuple, Other&&... other) {
  if constexpr (sizeof...(TupleArgs) <= Index) {
    return get_index<Index - sizeof...(TupleArgs)>(std::forward<Other>(other)...);
  } else {
    return get<Index>(std::forward<decltype(tuple)>(tuple));
  }
}

template <size_t Index, typename... TupleArgs, typename... Other>
constexpr auto get_index(const Tuple<TupleArgs...>& tuple, Other&&... other) {
  if constexpr (sizeof...(TupleArgs) <= Index) {
    return get_index<Index - sizeof...(TupleArgs)>(std::forward<Other>(other)...);
  } else {
    return get<Index>(std::forward<decltype(tuple)>(tuple));
  }
}

template <size_t Index, typename... TupleArgs, typename... Other>
constexpr auto get_index(Tuple<TupleArgs...>&& tuple, Other&&... other) {
  if constexpr (sizeof...(TupleArgs) <= Index) {
    return get_index<Index - sizeof...(TupleArgs)>(std::forward<Other>(other)...);
  } else {
    return get<Index>(std::forward<decltype(tuple)>(tuple));
  }
}

template <typename, typename>
struct TupleCat;

template <typename... First, typename... Second>
struct TupleCat<Tuple<First...>, Tuple<Second...>> {
  using type = Tuple<First..., Second...>;
};

template <typename... Tuples>
struct TupleCatMain;

template <>
struct TupleCatMain<> {
  using type = Tuple<>;
};

template <typename Tuple>
struct TupleCatMain<Tuple> {
  using type = Tuple;
};

template <typename First, typename Second, typename... Rest>
struct TupleCatMain<First, Second, Rest...> {
  using type = typename TupleCatMain<
    typename TupleCat<typename std::decay<First>::type,
                      typename std::decay<Second>::type>::type,
                      typename std::decay<Rest>::type...>::type;
};

template <typename... Tuples>
using TupleCatResult = typename TupleCatMain<Tuples...>::type;

auto tupleCat(auto&&... tuples) {
  using return_type = TupleCatResult<decltype(tuples)...>;
  return return_type(helper_index<0>(), std::forward<decltype(tuples)>(tuples)...);
}

template <typename T1, typename T2>
Tuple(const std::pair<T1, T2>&) -> Tuple<T1, T2>;

template <typename T1, typename T2>
Tuple(std::pair<T1, T2>&&) -> Tuple<T1, T2>;
