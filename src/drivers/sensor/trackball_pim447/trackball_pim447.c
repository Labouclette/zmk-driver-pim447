/* Diagnostic PIM447 : forcer la LED en bleu */
err = trackball_pim447_set_led(dev, 0, 100, 150);
    
if (err < 0)
{
    /* Ne pas démarrer le polling si l'I2C échoue */
    return err;
}

    data->dev = dev;
    k_work_init_delayable(&data->poll_work, trackball_pim447_poll);
    k_work_schedule(&data->poll_work, K_MSEC(TRACKBALL_PIM447_POLL_MS));

    LOG_INF("Pimoroni Trackball initialized (addr: 0x%02x)", config->i2c.addr);
    return 0;
}

static void trackball_pim447_poll(struct k_work *work)
{
    struct k_work_delayable *delayable =
        k_work_delayable_from_work(work);

    struct trackball_pim447_data *data =
        CONTAINER_OF(delayable, struct trackball_pim447_data, poll_work);

    const struct device *dev = data->dev;

    int16_t dx = 0;
    int16_t dy = 0;

    int err_x = trackball_pim447_read_axis(
        dev,
        TRACKBALL_PIM447_REG_LEFT,
        TRACKBALL_PIM447_REG_RIGHT,
        &dx
    );

    int err_y = trackball_pim447_read_axis(
        dev,
        TRACKBALL_PIM447_REG_UP,
        TRACKBALL_PIM447_REG_DOWN,
        &dy
    );

    if (err_x == 0 && err_y == 0) {

        int32_t move_x = dx;
        int32_t move_y = dy;

        if (data->invert_x) {
            move_x = -move_x;
        }

        if (data->invert_y) {
            move_y = -move_y;
        }

        move_x = move_x * data->sensitivity *
                 data->move_factor / 64;

        move_y = move_y * data->sensitivity *
                 data->move_factor / 64;

if (move_x != 0 || move_y != 0) {
    /* Axes permutés et sens vertical corrigé */
    input_report_rel(dev, INPUT_REL_X,
                     move_y, false, K_NO_WAIT);

    input_report_rel(dev, INPUT_REL_Y,
                     -move_x, true, K_NO_WAIT);
}
    }

    /* Le bit 7 du registre SWITCH indique l'appui physique. */
    uint8_t switch_state = 0;
    if (trackball_pim447_read_reg(dev, TRACKBALL_PIM447_REG_SWITCH,
                                  &switch_state) == 0) {
        bool pressed = (switch_state & 0x80U) != 0U;
        data->button_state = switch_state;

        if (pressed != data->button_pressed) {
            data->button_pressed = pressed;
            input_report_key(dev, INPUT_BTN_LEFT, pressed ? 1 : 0,
                             true, K_NO_WAIT);
        }
    }

    k_work_reschedule(
        &data->poll_work,
        K_MSEC(TRACKBALL_PIM447_POLL_MS)
    );
}

/* API functions structure */
static const struct sensor_driver_api trackball_pim447_api = {
    .sample_fetch = trackball_pim447_sample_fetch,
    .channel_get = trackball_pim447_channel_get,
    .attr_set = trackball_pim447_attr_set,
};

/* Driver initialization */
#define TRACKBALL_PIM447_INIT(inst)                                                       \
    static struct trackball_pim447_data trackball_pim447_data_##inst;                     \
                                                                                          \
    static const struct trackball_pim447_config trackball_pim447_config_##inst = {        \
        .i2c = I2C_DT_SPEC_INST_GET(inst),                                                \
        .led_red = DT_INST_PROP_OR(inst, led_red, 0),                                     \
        .led_green = DT_INST_PROP_OR(inst, led_green, 0),                                 \
        .led_blue = DT_INST_PROP_OR(inst, led_blue, 0),                                   \
        .invert_x = DT_INST_PROP_OR(inst, invert_x, false),                               \
        .invert_y = DT_INST_PROP_OR(inst, invert_y, false),                               \
        .sensitivity = DT_INST_PROP_OR(inst, sensitivity, 64),                            \
        .move_factor = DT_INST_PROP_OR(inst, move_factor, 1),                             \
        .scroll_factor = DT_INST_PROP_OR(inst, scroll_factor, 1),                         \
    };                                                                                    \
                                                                                          \
    DEVICE_DT_INST_DEFINE(inst, trackball_pim447_init, NULL,                              \
                          &trackball_pim447_data_##inst, &trackball_pim447_config_##inst, \
                          POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &trackball_pim447_api);

DT_INST_FOREACH_STATUS_OKAY(TRACKBALL_PIM447_INIT)
