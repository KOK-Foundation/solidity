{
  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let c := mload(3)    // c cannot be moved because non-movable
    let inv := add(b, c) // inv cannot be moved because its value depends on c
    a := add(a, 1)
    mstore(a, inv)
  }
}
// ====
// step: loopInvariantCodeMotion
// ----
// {
//     let b := 1
//     let a := 1
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let c := mload(3)
//         let inv := add(b, c)
//         a := add(a, 1)
//         mstore(a, inv)
//     }
// }
