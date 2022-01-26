#[repr(C)]
pub struct TickContext {
    pub variables: TickVariables,
    pub is_left_pressed: bool,
    pub is_right_pressed: bool,
    pub is_down_pressed: bool,
    pub is_rotate_pressed: bool,
}

#[repr(C)]
pub struct TickVariables {
    pub lcd_buffer: *mut u8, // has 80 length

    // temps
    // pain
    pub i: u8,
    pub j: u8,
    pub k: u8,
    pub l: u8,

    // states
    pub gaming: bool,
    pub tick_count: u8,
    pub button_state: u8,

    // randomizer
    pub rand_x: u8,
    pub rand_y: u8,
    pub rand_z: u8,

    pub rand_t: u8,
}

impl Default for TickVariables {
    fn default() -> Self {
        Self {
            lcd_buffer: 0 as _,
            i: 0,
            j: 0,
            k: 0,
            l: 0,
            tick_count: 0,
            button_state: 0,
            gaming: false,
            rand_x: 1,
            rand_y: 2,
            rand_z: 3,
            rand_t: 4,
        }
    }
}

#[no_mangle]
pub extern "C" fn _dummy(_: TickContext) {}
