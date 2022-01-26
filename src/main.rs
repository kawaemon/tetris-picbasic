use sdl2::{
    event::Event,
    keyboard::Keycode,
    pixels::Color,
    rect::Rect,
    render::{Canvas, Texture, TextureCreator},
    ttf::Font,
    video::{Window, WindowContext},
};

use std::{collections::HashMap, time::Duration};

mod ffi;

const LCD_BACKGROUND: Color = Color::RGB(0x59, 0x98, 0x1A);

fn main() {
    let sdl = sdl2::init().unwrap();
    let video = sdl.video().unwrap();
    let ttf = sdl2::ttf::init().unwrap();

    let mut canvas = video
        .window("window", 800, 600)
        .opengl()
        .build()
        .unwrap()
        .into_canvas()
        .build()
        .unwrap();

    let texture_creator = canvas.texture_creator();
    let font = ttf.load_font("./JetBrainsMono-Medium.ttf", 32).unwrap();
    let mut font_renderer = FontRenderCache {
        font,
        tc: &texture_creator,
        cache: HashMap::new(),
    };

    let mut lcd_buffer = [['a' as u8; 20]; 4];
    let mut tickcontext = ffi::TickContext {
        variables: ffi::TickVariables::default(),
        is_left_pressed: false,
        is_right_pressed: false,
        is_rotate_pressed: false,
    };

    tickcontext.variables.lcd_buffer = &mut lcd_buffer as *mut _ as _;

    let mut event_pump = sdl.event_pump().unwrap();

    'main: loop {
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => break 'main,

                Event::KeyDown {
                    keycode: Some(Keycode::Left),
                    ..
                } => tickcontext.is_left_pressed = true,

                Event::KeyUp {
                    keycode: Some(Keycode::Left),
                    ..
                } => tickcontext.is_left_pressed = false,

                Event::KeyDown {
                    keycode: Some(Keycode::Right),
                    ..
                } => tickcontext.is_right_pressed = true,

                Event::KeyUp {
                    keycode: Some(Keycode::Right),
                    ..
                } => tickcontext.is_right_pressed = false,

                Event::KeyDown {
                    keycode: Some(Keycode::Up),
                    ..
                } => tickcontext.is_rotate_pressed = true,

                Event::KeyUp {
                    keycode: Some(Keycode::Up),
                    ..
                } => tickcontext.is_rotate_pressed = false,

                _ => {}
            }
        }

        canvas.set_draw_color(LCD_BACKGROUND);
        canvas.clear();

        extern "C" {
            fn tick(ctx: *mut ffi::TickContext);
        }

        unsafe {
            tick(&mut tickcontext as _);
        }

        font_renderer.render_buffer(&mut canvas, &lcd_buffer, (250, 40));

        canvas.present();
        std::thread::sleep(Duration::from_millis(1000 / 60));
    }
}

struct FontRenderCache<'tc> {
    font: Font<'tc, 'static>,
    tc: &'tc TextureCreator<WindowContext>,
    cache: HashMap<u8, (Texture<'tc>, (u32, u32))>,
}

impl<'tc> FontRenderCache<'tc> {
    fn render(&mut self, canvas: &mut Canvas<Window>, c: u8, pos: (i32, i32)) -> (u32, u32) {
        let (texture, size) = self.cache.entry(c).or_insert_with(|| {
            let surface = self
                .font
                .render_char(c as char)
                .blended(Color::BLACK)
                .unwrap();

            let size = surface.size();
            let texture = self.tc.create_texture_from_surface(&surface).unwrap();
            (texture, size)
        });

        canvas
            .copy_ex(
                texture,
                None,
                Rect::new(pos.0, pos.1, size.0, size.1),
                90.0,
                None,
                false,
                false,
            )
            .unwrap();

        *size
    }

    fn render_buffer(&mut self, canvas: &mut Canvas<Window>, c: &[[u8; 20]; 4], pos: (i32, i32)) {
        let mut x = pos.0 as i32;
        let mut y = pos.1 as i32;

        for col in 0..4 {
            let mut y_max = 0;

            for row in 0..20 {
                let drew = self.render(canvas, c[col][row], (x, y));
                y += drew.0 as i32;
                y_max = y_max.max(drew.1);
            }

            y = pos.1 as i32;
            x -= y_max as i32;
        }
    }
}
