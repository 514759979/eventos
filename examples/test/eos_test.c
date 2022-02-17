#include "eventos.h"
#include "event_def.h"
#include "unity.h"
#include <stdbool.h>

// data struct -----------------------------------------------------------------
typedef struct led_tag {
    eos_sm_t super;
    bool status;
    int count;
} led_t;

// state -----------------------------------------------------------------------
static eos_ret_t led_state_init(led_t * const me, eos_event_t const * const e);
static eos_ret_t led_state_on(led_t * const me, eos_event_t const * const e);
static eos_ret_t led_state_off(led_t * const me, eos_event_t const * const e);

// api -------------------------------------------------------------------------
static void led_init(led_t * const me, eos_u8_t priv, void *queue, eos_u32_t queue_size)
{
    me->status = EOS_False;
    me->count = 0;

    eos_sm_init(&me->super, priv, queue, queue_size, 0, 0);
    eos_sm_start(&me->super, EOS_STATE_CAST(led_state_init));
}

static int led_get_evt_count(led_t * const me)
{
    return me->count;
}

static void led_reset_evt_count(led_t * const me)
{
    me->count = 0;
}

// state function --------------------------------------------------------------
static eos_ret_t led_state_init(led_t * const me, eos_event_t const * const e)
{
    (void)e;

    EOS_EVENT_SUB(Event_Time_500ms);

    eos_event_pub_period(Event_Time_500ms, 500);

    return EOS_TRAN(led_state_off);
}

static eos_ret_t led_state_off(led_t * const me, eos_event_t const * const e)
{
    switch (e->topic) {
        case Event_Enter:
            me->status = EOS_False;
            return EOS_Ret_Handled;

        case Event_Time_500ms:
            me->count ++;
            return EOS_TRAN(led_state_on);

        default:
            return EOS_SUPER(eos_state_top);
    }
}

static eos_ret_t led_state_on(led_t * const me, eos_event_t const * const e)
{
    switch (e->topic) {
        case Event_Enter: {
            int num = 1;
            me->status = EOS_True;
            return EOS_Ret_Handled;
        }

        case Event_Exit:
            return EOS_Ret_Handled;

        case Event_Time_500ms:
            me->count ++;
            return EOS_TRAN(led_state_off);

        default:
            return EOS_SUPER(eos_state_top);
    }
}

/* unittest ----------------------------------------------------------------- */
typedef struct eos_event_timer_tag {
    int topic;
    int time_ms_delay;
    eos_u32_t timeout_ms;
    bool statuse_shoot;
} eos_event_timer_t;

typedef struct frame_tag {
    eos_u32_t magic;
    eos_u32_t *evt_sub_tab;                                 // 事件订阅表

    // 状态机池
    eos_s32_t flag_obj_exist;
    eos_s32_t flag_obj_enable;
    eos_sm_t * p_obj[EOS_SM_NUM_MAX];

    // 关于事件池
    eos_event_t e_pool[EOS_EPOOL_SIZE];                           // 事件池
    eos_u32_t flag_epool[EOS_EPOOL_SIZE / 32 + 1];             // 事件池标志位

    // 定时器池
    eos_event_timer_t e_timer_pool[EOS_ETIMERPOOL_SIZE];
    eos_u32_t flag_etimerpool[EOS_ETIMERPOOL_SIZE / 32 + 1];   // 事件池标志位
    eos_bool_t is_etimerpool_empty;
    eos_u32_t timeout_ms_min;

    eos_bool_t is_enabled;
    eos_bool_t is_running;
    eos_bool_t is_idle;
    
    eos_u32_t time_crt_ms;
} frame_t;

extern int eos_once(void);
extern void * eos_get_framework(void);
extern int eos_evttimer(void);
extern void set_time_ms(uint64_t time_ms);

static eos_u32_t eos_sub_table[Event_Max];

void eos_test(void)
{
    frame_t * f = (frame_t *)eos_get_framework();
    // -------------------------------------------------------------------------
    // 01 meow_init
    TEST_ASSERT_EQUAL_INT32(1, eos_once());
    // 检查事件池标志位，事件定时器标志位和事件申请区的标志位。
    eventos_init(eos_sub_table);
    TEST_ASSERT_EQUAL_INT8(EOS_True, f->is_etimerpool_empty);

    uint32_t _flag_epool[EOS_EPOOL_SIZE / 32 + 1] = {
        0, 0, 0, 0, 0xfffffffc
    };
    for (int i = 0; i < (EOS_EPOOL_SIZE / 32 + 1); i ++) {
        TEST_ASSERT_EQUAL_INT32(_flag_epool[i], f->flag_epool[i]);
    }
    uint32_t _flag_etimerpool[EOS_ETIMERPOOL_SIZE / 32 + 1] = {
        0, 0, 0xffffffc0
    };
    TEST_ASSERT_EQUAL_INT8(EOS_True, f->is_etimerpool_empty);
    for (int i = 0; i < (EOS_ETIMERPOOL_SIZE / 32 + 1); i ++) {
        TEST_ASSERT_EQUAL_INT32(_flag_etimerpool[i], f->flag_etimerpool[i]);
    }
    TEST_ASSERT_EQUAL_INT8(EOS_True, f->is_enabled);
    TEST_ASSERT_EQUAL_INT8(EOS_True, f->is_idle);

    // -------------------------------------------------------------------------
    // 02 meow_stop
    TEST_ASSERT_EQUAL_INT32(201, eos_once());
    eventos_stop();
    TEST_ASSERT_EQUAL_INT32(1, eos_once());
    TEST_ASSERT_EQUAL_INT32(210, eos_evttimer());

    // -------------------------------------------------------------------------
    // 03 sm_init sm_start
    eventos_init(eos_sub_table);

    static led_t led_test, led2;
    static eos_u32_t queue_test[130], queue2[130];
    TEST_ASSERT_EQUAL_UINT32(0, led_test.super.priv);
    led_init(&led_test, 1, queue_test, 130);
    TEST_ASSERT_EQUAL_UINT32(1, led_test.super.priv);

    led_init(&led2, 0, queue2, 130);
    TEST_ASSERT_EQUAL_INT8(EOS_False, f->is_etimerpool_empty);
    for (int i = 0; i < 10; i ++) {
        TEST_ASSERT_EQUAL_INT32(202, eos_once());
    }
    // -------------------------------------------------------------------------
    // 04 sm_init sm_start
    eos_event_pub_delay(Event_Time_500ms, 0);
    // 检查事件池
    TEST_ASSERT_EQUAL_UINT32(1, f->flag_epool[0]);
    // 检查每个状态机的事件队列
    TEST_ASSERT_EQUAL_INT8(EOS_False, led_test.super.equeue_empty);
    TEST_ASSERT_EQUAL_UINT8(1, led_test.super.priv);
    TEST_ASSERT_EQUAL_UINT32(0, led_test.super.tail);
    TEST_ASSERT_EQUAL_UINT32(1, led_test.super.head);
    TEST_ASSERT_EQUAL_INT8(EOS_False, led2.super.equeue_empty);
    TEST_ASSERT_EQUAL_UINT8(0, led2.super.priv);
    TEST_ASSERT_EQUAL_UINT32(0, led2.super.tail);
    TEST_ASSERT_EQUAL_UINT32(1, led2.super.head);
    // 优先级高的状态机，消费掉此事件。
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_UINT8(EOS_False, led_test.super.equeue_empty);
    TEST_ASSERT_EQUAL_UINT32(1, led_test.super.head);
    TEST_ASSERT_EQUAL_UINT32(0, led_test.super.tail);
    TEST_ASSERT_EQUAL_UINT8(EOS_True, led2.super.equeue_empty);
    TEST_ASSERT_EQUAL_UINT32(0, led2.super.head);
    TEST_ASSERT_EQUAL_UINT32(0, led2.super.tail);
    // 优先级低的状态机，消费掉此事件。
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_UINT8(EOS_True, led_test.super.equeue_empty);
    TEST_ASSERT_EQUAL_UINT32(0, led_test.super.head);
    TEST_ASSERT_EQUAL_UINT32(0, led_test.super.tail);
    TEST_ASSERT_EQUAL_UINT32(EOS_True, led2.super.equeue_empty);
    TEST_ASSERT_EQUAL_UINT32(0, led2.super.head);
    TEST_ASSERT_EQUAL_UINT32(0, led2.super.tail);
    TEST_ASSERT_EQUAL_INT32(203, eos_once());
    TEST_ASSERT_EQUAL_INT32(202, eos_once());
    TEST_ASSERT_EQUAL_UINT32(0, f->flag_epool[0]);
    
    // -------------------------------------------------------------------------
    // 04 eos_event_pub_topic
    eos_event_pub_topic(Event_Time_500ms);
    TEST_ASSERT_EQUAL_UINT32(1, f->flag_epool[0]);
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(203, eos_once());
    TEST_ASSERT_EQUAL_INT32(202, eos_once());
    TEST_ASSERT_EQUAL_UINT32(0, f->flag_epool[0]);
    int evt_num = EOS_EPOOL_SIZE - 1;
    // 测试事件池满的情形
    for (int i = 0; i < evt_num; i ++) {
        eos_event_pub_topic(Event_Time_500ms);
        TEST_ASSERT_NOT_EQUAL_UINT32(0, f->flag_epool[i / 32] & (1 << (i % 32)));
        TEST_ASSERT_EQUAL_UINT32((i + 1), led2.super.head);
        TEST_ASSERT_EQUAL_UINT32(i, *(led2.super.e_queue + i));
    }

    // 事件池满，测试通过
    // eos_event_pub_topic(Event_Time_500ms);
    // 优先级高的状态机执行其事件。
    for (int i = 0; i < evt_num; i ++) {
        TEST_ASSERT_EQUAL_INT32(0, eos_once());
    }

    // 优先级低的状态机执行其事件。
    for (int i = 0; i < evt_num; i ++) {
        TEST_ASSERT_EQUAL_INT32(0, eos_once());
        TEST_ASSERT_EQUAL_UINT32(0, (f->flag_epool[i / 32] & (1 << (i % 32))));
    }
    TEST_ASSERT_EQUAL_INT32(203, eos_once());
    TEST_ASSERT_EQUAL_INT32(202, eos_once());
    for (int i = 0; i < (EOS_EPOOL_SIZE / 32 + 1); i ++) {
        TEST_ASSERT_EQUAL_UINT32(_flag_epool[i], f->flag_epool[i]);
    }

    // -------------------------------------------------------------------------
    // 05 evt_unsubscribe
    eos_event_pub_topic(Event_Time_500ms);
    eos_event_pub_topic(Event_Time_500ms);
    eos_event_unsub(&led2.super, Event_Time_500ms);
    TEST_ASSERT_EQUAL_UINT32(204, eos_once());
    TEST_ASSERT_EQUAL_UINT32(204, eos_once());
    TEST_ASSERT_EQUAL_UINT32(0, eos_once());
    TEST_ASSERT_EQUAL_UINT32(0, eos_once());
    TEST_ASSERT_EQUAL_UINT32(203, eos_once());
    TEST_ASSERT_EQUAL_UINT32(202, eos_once());
    TEST_ASSERT_EQUAL_UINT32(0, (f->flag_epool[0]));

    // -------------------------------------------------------------------------
    // 06 eos_event_pub_topiclish_delay
    eos_event_sub(&led2.super, Event_Time_500ms);
    TEST_ASSERT_EQUAL_UINT32(1, f->flag_etimerpool[0]);
    TEST_ASSERT_EQUAL_UINT8(EOS_False, f->is_etimerpool_empty);
    TEST_ASSERT_EQUAL_UINT32(500, f->e_timer_pool[0].timeout_ms);
    set_time_ms(200);
    TEST_ASSERT_EQUAL_INT32(211, eos_evttimer());
    set_time_ms(500);
    TEST_ASSERT_EQUAL_INT32(0, eos_evttimer());
    TEST_ASSERT_EQUAL_UINT32(500, f->time_crt_ms);
    TEST_ASSERT_EQUAL_UINT32(1000, f->e_timer_pool[0].timeout_ms);
    set_time_ms(1000);
    TEST_ASSERT_EQUAL_INT32(0, eos_evttimer());
    TEST_ASSERT_EQUAL_UINT32(1000, f->time_crt_ms);
    TEST_ASSERT_EQUAL_UINT32(1500, f->e_timer_pool[0].timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(3, f->flag_epool[0]);
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(203, eos_once());
    TEST_ASSERT_EQUAL_INT32(202, eos_once());

    // 检查重复时间定时器的重复设定
    // eos_event_pub_topiclish_period(Event_Time_500ms, 200);

    int count_etimer = (EOS_ETIMERPOOL_SIZE - 1);
    // 检查时间定时器满
    for (int i = 0; i < count_etimer; i ++) {
        eos_event_pub_delay(Event_Time_500ms, 200);
    }
    // eos_event_pub_topiclish_delay(Event_Time_500ms, 200);

    // 检查事件定时器发送出的延时事件的执行
    set_time_ms(1200);
    led_reset_evt_count(&led_test);
    led_reset_evt_count(&led2);
    for (int i = 0; i < count_etimer; i ++) {
        TEST_ASSERT_EQUAL_INT32(0, eos_once());
    }
    for (int i = 0; i < count_etimer; i ++) {
        TEST_ASSERT_EQUAL_INT32(0, eos_once());
    }
    TEST_ASSERT_EQUAL_INT32(203, eos_once());
    TEST_ASSERT_EQUAL_INT32(202, eos_once());
    TEST_ASSERT_EQUAL_UINT32(count_etimer, led_get_evt_count(&led_test));
    TEST_ASSERT_EQUAL_UINT32(count_etimer, led_get_evt_count(&led2));

    // 周期延时事件的执行
    set_time_ms(1500);
    led_reset_evt_count(&led_test);
    led_reset_evt_count(&led2);
    // 一个状态机的执行
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_UINT32(0, led_get_evt_count(&led_test));
    TEST_ASSERT_EQUAL_UINT32(1, led_get_evt_count(&led2));
    // 另一个状态机的执行
    TEST_ASSERT_EQUAL_INT32(0, eos_once());
    TEST_ASSERT_EQUAL_INT32(203, eos_once());
    TEST_ASSERT_EQUAL_INT32(202, eos_once());
    TEST_ASSERT_EQUAL_UINT32(1, led_get_evt_count(&led_test));
    TEST_ASSERT_EQUAL_UINT32(1, led_get_evt_count(&led2));
    TEST_ASSERT_EQUAL_UINT32(1500, f->time_crt_ms);
    TEST_ASSERT_EQUAL_UINT32(2000, f->e_timer_pool[0].timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(0, f->flag_epool[0]);

    TEST_ASSERT_EQUAL_INT32(202, eos_once());
}