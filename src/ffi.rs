#[repr(C)]
pub struct TickContext {
    pub variables: TickVariables,
}

#[repr(C)]
pub struct TickVariables {
    pub lcd_buffer: *mut u8, // has 80 length
}

#[no_mangle]
pub extern "C" fn _dummy(_: TickContext) {}
