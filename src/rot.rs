use super::ffi;

#[allow(non_camel_case_types)]
type int = u8;

const NEG_OFFSET: u8 = 50;

fn convert_between_math_coordinate(i: int, length: int) -> int {
    length - i - 1
}

fn convert_into_relative_coordinate(i: int, c: int) -> int {
    let mut res = NEG_OFFSET + i - c;
    if i >= c {
        res += 1
    }
    res
}

fn convert_from_relative_coordinate(i: int, c: int) -> int {
    let mut res = i + c;
    if NEG_OFFSET <= i {
        res -= 1
    }
    res - NEG_OFFSET
}

const WIDTH: int = 4;
const HEIGHT: int = 9;

// minimized version of original. not intented to be read :D
fn test_me((x, y): (int, int), (center_x, center_y): (int, int)) -> (int, int) {
    let rotated_x = {
        let mut r_x = NEG_OFFSET - y - 1 + center_y;
        if WIDTH - y - 1 >= WIDTH - center_y {
            r_x += 1;
        }
        let mut rotated_m_y = r_x + HEIGHT - center_x;
        if NEG_OFFSET <= r_x {
            rotated_m_y -= 1;
        }
        NEG_OFFSET + HEIGHT - rotated_m_y - 1
    };

    let rotated_y = {
        let mut r_y = NEG_OFFSET - x + center_x - 1;
        if HEIGHT - x - 1 >= HEIGHT - center_x {
            r_y += 1;
        }

        let rotated_r_x; // = -r_y;
        if r_y < NEG_OFFSET {
            // r_y is negative
            rotated_r_x = r_y + 2 * (NEG_OFFSET - r_y);
        } else {
            // r_y is positive
            rotated_r_x = r_y - 2 * (r_y - NEG_OFFSET);
        }

        let mut rotated_m_x = rotated_r_x + WIDTH - center_y;
        if NEG_OFFSET <= rotated_r_x {
            rotated_m_x -= 1;
        }
        NEG_OFFSET + WIDTH - rotated_m_x - 1
    };

    (rotated_x, rotated_y)
}

fn original((x, y): (int, int), (center_x, center_y): (int, int)) -> (int, int) {
    let m_x = convert_between_math_coordinate(y, WIDTH);
    let m_y = convert_between_math_coordinate(x, HEIGHT);
    let m_center_x = convert_between_math_coordinate(center_y, WIDTH + 1);
    let m_center_y = convert_between_math_coordinate(center_x, HEIGHT + 1);

    let r_x = convert_into_relative_coordinate(m_x, m_center_x);
    let r_y = convert_into_relative_coordinate(m_y, m_center_y);

    let rotated_r_x; // = -r_y;
    if r_y < NEG_OFFSET {
        // r_y is negative
        rotated_r_x = r_y + 2 * (NEG_OFFSET - r_y);
    } else {
        // r_y is positive
        rotated_r_x = r_y - 2 * (r_y - NEG_OFFSET);
    }

    let rotated_r_y = r_x;

    let rotated_m_x = convert_from_relative_coordinate(rotated_r_x, m_center_x);
    let rotated_m_y = convert_from_relative_coordinate(rotated_r_y, m_center_y);

    let rotated_x = convert_between_math_coordinate(rotated_m_y, HEIGHT);
    let rotated_y = convert_between_math_coordinate(rotated_m_x, WIDTH);

    (rotated_x, rotated_y)
}

#[test]
fn test_test_me() {
    assert_eq!(test_me((2, 1), (3, 2)), (2, 2));
    assert_eq!(test_me((1, 0), (3, 2)), (1, 3));
    assert_eq!(test_me((4, 2), (3, 2)), (3, 0));
}

#[test]
fn test_original() {
    assert_eq!(original((2, 1), (3, 2)), (2, 2));
    assert_eq!(original((1, 0), (3, 2)), (1, 3));
    assert_eq!(original((4, 2), (3, 2)), (3, 0));
}

#[test]
fn test_test_me_c() {
    assert_eq!(test_test_me_helper((2, 1), (3, 2)), (2, 2));
    assert_eq!(test_test_me_helper((1, 0), (3, 2)), (1, 3));
    assert_eq!(test_test_me_helper((4, 2), (3, 2)), (3, 0));
}

fn test_test_me_helper((x, y): (int, int), (center_x, center_y): (int, int)) -> (int, int) {
    let mut v = ffi::TickVariables {
        i: x,
        j: y,
        rot_center_x: center_x,
        rot_center_y: center_y,
        ..ffi::TickVariables::default()
    };

    extern "C" {
        fn rotate_pos(v: *mut ffi::TickVariables);
    }

    unsafe { rotate_pos(&mut v as _) };

    (v.k, v.l)
}

#[test]
fn test_convert_between_math_coordinate() {
    // from
    assert_eq!(convert_between_math_coordinate(1, 4), 2);
    assert_eq!(convert_between_math_coordinate(2, 9), 6);

    // into
    assert_eq!(convert_between_math_coordinate(1, 4), 2);
    assert_eq!(convert_between_math_coordinate(6, 9), 2);
    assert_eq!(convert_between_math_coordinate(1, 9), 7);

    assert_eq!(convert_between_math_coordinate(2, 9), 6);

    // boundary from
    assert_eq!(convert_between_math_coordinate(2, 9), 6);

    // boundary into
    assert_eq!(convert_between_math_coordinate(3, 9), 5);
}
#[test]
fn test_convert_into_relative_coordinate() {
    assert_eq!(convert_into_relative_coordinate(2, 2), NEG_OFFSET + 1);
    assert_eq!(convert_into_relative_coordinate(6, 6), NEG_OFFSET + 1);
    assert_eq!(convert_into_relative_coordinate(5, 6), NEG_OFFSET - 1);
    assert_eq!(convert_into_relative_coordinate(3, 2), NEG_OFFSET + 2);
}
#[test]
fn test_convert_from_relative_coordinate() {
    assert_eq!(convert_from_relative_coordinate(1 + NEG_OFFSET, 6), 6);
    assert_eq!(convert_from_relative_coordinate(NEG_OFFSET - 2, 6), 4);
}
