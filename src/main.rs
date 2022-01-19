use sdl2:: {
    pixels::Color,
    event::Event,
    keyboard::Keycode,
};

use std::{
    time::Duration
};

fn main() {
    let sdl = sdl2::init().unwrap();
    let video = sdl.video().unwrap();
    let mut canvas = video.window("window", 800, 600)
        .opengl()
        .build()
        .unwrap()
        .into_canvas()
        .build()
        .unwrap();
    
    const LCD_BACKGROUND: Color = Color::RGB(0x59, 0x98, 0x1A);

    let mut event_pump = sdl.event_pump().unwrap();

    'main: loop {
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. } |
                    Event::KeyDown { keycode: Some(Keycode::Escape), ..} => break 'main,
                _ => {}
            }
        }

        canvas.set_draw_color(LCD_BACKGROUND);
        canvas.clear();
        canvas.present();

        std::thread::sleep(Duration::from_millis(100));
    }
}
