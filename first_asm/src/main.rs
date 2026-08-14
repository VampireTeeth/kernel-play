// Declare the external assembly function
unsafe extern "C" {
    fn add_function(a: i64, b: i64) -> i64;
}

fn main() {
    unsafe {
        let r = add_function(3, 5);
        println!("The result from ASM is: {}", r);
    }
}