{
	let a := origin()
	let b := calldataload(0)
	let i := 0
	for {} lt(i, 10) {i := add(a, b)} {
	}
}
// ====
// step: rematerialiser
// ----
// {
//     let a := origin()
//     let b := calldataload(0)
//     let i := 0
//     for { } lt(i, 10) { i := add(origin(), b) }
//     { }
// }
