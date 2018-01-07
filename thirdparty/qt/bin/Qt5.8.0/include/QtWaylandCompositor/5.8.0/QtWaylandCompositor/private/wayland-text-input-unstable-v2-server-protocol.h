/* 
 * Copyright © 2012, 2013 Intel Corporation
 * Copyright © 2015, 2016 Jan Arne Petersen
 * 
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in
 * all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * the copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 * 
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef TEXT_INPUT_UNSTABLE_V2_SERVER_PROTOCOL_H
#define TEXT_INPUT_UNSTABLE_V2_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

struct wl_client;
struct wl_resource;

struct wl_seat;
struct wl_surface;
struct zwp_text_input_manager_v2;
struct zwp_text_input_v2;

extern const struct wl_interface zwp_text_input_v2_interface;
extern const struct wl_interface zwp_text_input_manager_v2_interface;

#ifndef ZWP_TEXT_INPUT_V2_CONTENT_HINT_ENUM
#define ZWP_TEXT_INPUT_V2_CONTENT_HINT_ENUM
/**
 * zwp_text_input_v2_content_hint - content hint
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_NONE: no special behaviour
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_COMPLETION: suggest word
 *	completions
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_CORRECTION: suggest word
 *	corrections
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_CAPITALIZATION: switch to
 *	uppercase letters at the start of a sentence
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_LOWERCASE: prefer lowercase letters
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_UPPERCASE: prefer uppercase letters
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_TITLECASE: prefer casing for titles
 *	and headings (can be language dependent)
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_HIDDEN_TEXT: characters should be
 *	hidden
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_SENSITIVE_DATA: typed text should not
 *	be stored
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_LATIN: just latin characters should be
 *	entered
 * @ZWP_TEXT_INPUT_V2_CONTENT_HINT_MULTILINE: the text input is multiline
 *
 * Content hint is a bitmask to allow to modify the behavior of the text
 * input.
 */
enum zwp_text_input_v2_content_hint {
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_NONE = 0x0,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_COMPLETION = 0x1,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_CORRECTION = 0x2,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_CAPITALIZATION = 0x4,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_LOWERCASE = 0x8,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_UPPERCASE = 0x10,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_TITLECASE = 0x20,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_HIDDEN_TEXT = 0x40,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_SENSITIVE_DATA = 0x80,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_LATIN = 0x100,
	ZWP_TEXT_INPUT_V2_CONTENT_HINT_MULTILINE = 0x200,
};
#endif /* ZWP_TEXT_INPUT_V2_CONTENT_HINT_ENUM */

#ifndef ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_ENUM
#define ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_ENUM
/**
 * zwp_text_input_v2_content_purpose - content purpose
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NORMAL: default input, allowing all
 *	characters
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_ALPHA: allow only alphabetic
 *	characters
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DIGITS: allow only digits
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NUMBER: input a number (including
 *	decimal separator and sign)
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_PHONE: input a phone number
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_URL: input an URL
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_EMAIL: input an email address
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NAME: input a name of a person
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_PASSWORD: input a password (combine
 *	with password or sensitive_data hint)
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DATE: input a date
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_TIME: input a time
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DATETIME: input a date and time
 * @ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_TERMINAL: input for a terminal
 *
 * The content purpose allows to specify the primary purpose of a text
 * input.
 *
 * This allows an input method to show special purpose input panels with
 * extra characters or to disallow some characters.
 */
enum zwp_text_input_v2_content_purpose {
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NORMAL = 0,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_ALPHA = 1,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DIGITS = 2,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NUMBER = 3,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_PHONE = 4,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_URL = 5,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_EMAIL = 6,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NAME = 7,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_PASSWORD = 8,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DATE = 9,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_TIME = 10,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DATETIME = 11,
	ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_TERMINAL = 12,
};
#endif /* ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_ENUM */

#ifndef ZWP_TEXT_INPUT_V2_UPDATE_STATE_ENUM
#define ZWP_TEXT_INPUT_V2_UPDATE_STATE_ENUM
/**
 * zwp_text_input_v2_update_state - update_state flags
 * @ZWP_TEXT_INPUT_V2_UPDATE_STATE_CHANGE: updated state because it
 *	changed
 * @ZWP_TEXT_INPUT_V2_UPDATE_STATE_FULL: full state after enter or
 *	input_method_changed event
 * @ZWP_TEXT_INPUT_V2_UPDATE_STATE_RESET: full state after reset
 * @ZWP_TEXT_INPUT_V2_UPDATE_STATE_ENTER: full state after switching
 *	focus to a different widget on client side
 *
 * Defines the reason for sending an updated state.
 */
enum zwp_text_input_v2_update_state {
	ZWP_TEXT_INPUT_V2_UPDATE_STATE_CHANGE = 0,
	ZWP_TEXT_INPUT_V2_UPDATE_STATE_FULL = 1,
	ZWP_TEXT_INPUT_V2_UPDATE_STATE_RESET = 2,
	ZWP_TEXT_INPUT_V2_UPDATE_STATE_ENTER = 3,
};
#endif /* ZWP_TEXT_INPUT_V2_UPDATE_STATE_ENUM */

#ifndef ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_ENUM
#define ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_ENUM
enum zwp_text_input_v2_input_panel_visibility {
	ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_HIDDEN = 0,
	ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_VISIBLE = 1,
};
#endif /* ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_ENUM */

#ifndef ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_ENUM
#define ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_ENUM
enum zwp_text_input_v2_preedit_style {
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_DEFAULT = 0,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_NONE = 1,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_ACTIVE = 2,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_INACTIVE = 3,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_HIGHLIGHT = 4,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_UNDERLINE = 5,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_SELECTION = 6,
	ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_INCORRECT = 7,
};
#endif /* ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_ENUM */

#ifndef ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_ENUM
#define ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_ENUM
enum zwp_text_input_v2_text_direction {
	ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_AUTO = 0,
	ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_LTR = 1,
	ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_RTL = 2,
};
#endif /* ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_ENUM */

/**
 * zwp_text_input_v2 - text input
 * @destroy: Destroy the wp_text_input
 * @enable: enable text input for surface
 * @disable: disable text input for surface
 * @show_input_panel: show input panels
 * @hide_input_panel: hide input panels
 * @set_surrounding_text: sets the surrounding text
 * @set_content_type: set content purpose and hint
 * @set_cursor_rectangle: set cursor position
 * @set_preferred_language: sets preferred language
 * @update_state: update state
 *
 * The zwp_text_input_v2 interface represents text input and input
 * methods associated with a seat. It provides enter/leave events to follow
 * the text input focus for a seat.
 *
 * Requests are used to enable/disable the text-input object and set state
 * information like surrounding and selected text or the content type. The
 * information about the entered text is sent to the text-input object via
 * the pre-edit and commit events. Using this interface removes the need
 * for applications to directly process hardware key events and compose
 * text out of them.
 *
 * Text is valid UTF-8 encoded, indices and lengths are in bytes. Indices
 * have to always point to the first byte of an UTF-8 encoded code point.
 * Lengths are not allowed to contain just a part of an UTF-8 encoded code
 * point.
 *
 * State is sent by the state requests (set_surrounding_text,
 * set_content_type, set_cursor_rectangle and set_preferred_language) and
 * an update_state request. After an enter or an input_method_change event
 * all state information is invalidated and needs to be resent from the
 * client. A reset or entering a new widget on client side also invalidates
 * all current state information.
 */
struct zwp_text_input_v2_interface {
	/**
	 * destroy - Destroy the wp_text_input
	 *
	 * Destroy the wp_text_input object. Also disables all surfaces
	 * enabled through this wp_text_input object
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * enable - enable text input for surface
	 * @surface: (none)
	 *
	 * Enable text input in a surface (usually when a text entry
	 * inside of it has focus).
	 *
	 * This can be called before or after a surface gets text (or
	 * keyboard) focus via the enter event. Text input to a surface is
	 * only active when it has the current text (or keyboard) focus and
	 * is enabled.
	 */
	void (*enable)(struct wl_client *client,
		       struct wl_resource *resource,
		       struct wl_resource *surface);
	/**
	 * disable - disable text input for surface
	 * @surface: (none)
	 *
	 * Disable text input in a surface (typically when there is no
	 * focus on any text entry inside the surface).
	 */
	void (*disable)(struct wl_client *client,
			struct wl_resource *resource,
			struct wl_resource *surface);
	/**
	 * show_input_panel - show input panels
	 *
	 * Requests input panels (virtual keyboard) to show.
	 *
	 * This should be used for example to show a virtual keyboard again
	 * (with a tap) after it was closed by pressing on a close button
	 * on the keyboard.
	 */
	void (*show_input_panel)(struct wl_client *client,
				 struct wl_resource *resource);
	/**
	 * hide_input_panel - hide input panels
	 *
	 * Requests input panels (virtual keyboard) to hide.
	 */
	void (*hide_input_panel)(struct wl_client *client,
				 struct wl_resource *resource);
	/**
	 * set_surrounding_text - sets the surrounding text
	 * @text: (none)
	 * @cursor: (none)
	 * @anchor: (none)
	 *
	 * Sets the plain surrounding text around the input position.
	 * Text is UTF-8 encoded. Cursor is the byte offset within the
	 * surrounding text. Anchor is the byte offset of the selection
	 * anchor within the surrounding text. If there is no selected
	 * text, anchor is the same as cursor.
	 *
	 * Make sure to always send some text before and after the cursor
	 * except when the cursor is at the beginning or end of text.
	 *
	 * When there was a configure_surrounding_text event take the
	 * before_cursor and after_cursor arguments into account for
	 * picking how much surrounding text to send.
	 *
	 * There is a maximum length of wayland messages so text can not be
	 * longer than 4000 bytes.
	 */
	void (*set_surrounding_text)(struct wl_client *client,
				     struct wl_resource *resource,
				     const char *text,
				     int32_t cursor,
				     int32_t anchor);
	/**
	 * set_content_type - set content purpose and hint
	 * @hint: (none)
	 * @purpose: (none)
	 *
	 * Sets the content purpose and content hint. While the purpose
	 * is the basic purpose of an input field, the hint flags allow to
	 * modify some of the behavior.
	 *
	 * When no content type is explicitly set, a normal content purpose
	 * with none hint should be assumed.
	 */
	void (*set_content_type)(struct wl_client *client,
				 struct wl_resource *resource,
				 uint32_t hint,
				 uint32_t purpose);
	/**
	 * set_cursor_rectangle - set cursor position
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * Sets the cursor outline as a x, y, width, height rectangle in
	 * surface local coordinates.
	 *
	 * Allows the compositor to put a window with word suggestions near
	 * the cursor.
	 */
	void (*set_cursor_rectangle)(struct wl_client *client,
				     struct wl_resource *resource,
				     int32_t x,
				     int32_t y,
				     int32_t width,
				     int32_t height);
	/**
	 * set_preferred_language - sets preferred language
	 * @language: (none)
	 *
	 * Sets a specific language. This allows for example a virtual
	 * keyboard to show a language specific layout. The "language"
	 * argument is a RFC-3066 format language tag.
	 *
	 * It could be used for example in a word processor to indicate
	 * language of currently edited document or in an instant message
	 * application which tracks languages of contacts.
	 */
	void (*set_preferred_language)(struct wl_client *client,
				       struct wl_resource *resource,
				       const char *language);
	/**
	 * update_state - update state
	 * @serial: serial of the enter or input_method_changed event
	 * @reason: (none)
	 *
	 * Allows to atomically send state updates from client.
	 *
	 * This request should follow after a batch of state updating
	 * requests like set_surrounding_text, set_content_type,
	 * set_cursor_rectangle and set_preferred_language.
	 *
	 * The flags field indicates why an updated state is sent to the
	 * input method.
	 *
	 * Reset should be used by an editor widget after the text was
	 * changed outside of the normal input method flow.
	 *
	 * For "change" it is enough to send the changed state, else the
	 * full state should be send.
	 *
	 * Serial should be set to the serial from the last enter or
	 * input_method_changed event.
	 *
	 * To make sure to not receive outdated input method events after a
	 * reset or switching to a new widget wl_display_sync() should be
	 * used after update_state in these cases.
	 */
	void (*update_state)(struct wl_client *client,
			     struct wl_resource *resource,
			     uint32_t serial,
			     uint32_t reason);
};

#define ZWP_TEXT_INPUT_V2_ENTER	0
#define ZWP_TEXT_INPUT_V2_LEAVE	1
#define ZWP_TEXT_INPUT_V2_INPUT_PANEL_STATE	2
#define ZWP_TEXT_INPUT_V2_PREEDIT_STRING	3
#define ZWP_TEXT_INPUT_V2_PREEDIT_STYLING	4
#define ZWP_TEXT_INPUT_V2_PREEDIT_CURSOR	5
#define ZWP_TEXT_INPUT_V2_COMMIT_STRING	6
#define ZWP_TEXT_INPUT_V2_CURSOR_POSITION	7
#define ZWP_TEXT_INPUT_V2_DELETE_SURROUNDING_TEXT	8
#define ZWP_TEXT_INPUT_V2_MODIFIERS_MAP	9
#define ZWP_TEXT_INPUT_V2_KEYSYM	10
#define ZWP_TEXT_INPUT_V2_LANGUAGE	11
#define ZWP_TEXT_INPUT_V2_TEXT_DIRECTION	12
#define ZWP_TEXT_INPUT_V2_CONFIGURE_SURROUNDING_TEXT	13
#define ZWP_TEXT_INPUT_V2_INPUT_METHOD_CHANGED	14

#define ZWP_TEXT_INPUT_V2_ENTER_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_LEAVE_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_INPUT_PANEL_STATE_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_PREEDIT_STRING_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_PREEDIT_STYLING_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_PREEDIT_CURSOR_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_COMMIT_STRING_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_CURSOR_POSITION_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_DELETE_SURROUNDING_TEXT_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_MODIFIERS_MAP_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_KEYSYM_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_LANGUAGE_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_CONFIGURE_SURROUNDING_TEXT_SINCE_VERSION	1
#define ZWP_TEXT_INPUT_V2_INPUT_METHOD_CHANGED_SINCE_VERSION	1

static inline void
zwp_text_input_v2_send_enter(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_ENTER, serial, surface);
}

static inline void
zwp_text_input_v2_send_leave(struct wl_resource *resource_, uint32_t serial, struct wl_resource *surface)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_LEAVE, serial, surface);
}

static inline void
zwp_text_input_v2_send_input_panel_state(struct wl_resource *resource_, uint32_t state, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_INPUT_PANEL_STATE, state, x, y, width, height);
}

static inline void
zwp_text_input_v2_send_preedit_string(struct wl_resource *resource_, const char *text, const char *commit)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_PREEDIT_STRING, text, commit);
}

static inline void
zwp_text_input_v2_send_preedit_styling(struct wl_resource *resource_, uint32_t index, uint32_t length, uint32_t style)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_PREEDIT_STYLING, index, length, style);
}

static inline void
zwp_text_input_v2_send_preedit_cursor(struct wl_resource *resource_, int32_t index)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_PREEDIT_CURSOR, index);
}

static inline void
zwp_text_input_v2_send_commit_string(struct wl_resource *resource_, const char *text)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_COMMIT_STRING, text);
}

static inline void
zwp_text_input_v2_send_cursor_position(struct wl_resource *resource_, int32_t index, int32_t anchor)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_CURSOR_POSITION, index, anchor);
}

static inline void
zwp_text_input_v2_send_delete_surrounding_text(struct wl_resource *resource_, uint32_t before_length, uint32_t after_length)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_DELETE_SURROUNDING_TEXT, before_length, after_length);
}

static inline void
zwp_text_input_v2_send_modifiers_map(struct wl_resource *resource_, struct wl_array *map)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_MODIFIERS_MAP, map);
}

static inline void
zwp_text_input_v2_send_keysym(struct wl_resource *resource_, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_KEYSYM, time, sym, state, modifiers);
}

static inline void
zwp_text_input_v2_send_language(struct wl_resource *resource_, const char *language)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_LANGUAGE, language);
}

static inline void
zwp_text_input_v2_send_text_direction(struct wl_resource *resource_, uint32_t direction)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_TEXT_DIRECTION, direction);
}

static inline void
zwp_text_input_v2_send_configure_surrounding_text(struct wl_resource *resource_, int32_t before_cursor, int32_t after_cursor)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_CONFIGURE_SURROUNDING_TEXT, before_cursor, after_cursor);
}

static inline void
zwp_text_input_v2_send_input_method_changed(struct wl_resource *resource_, uint32_t serial, uint32_t flags)
{
	wl_resource_post_event(resource_, ZWP_TEXT_INPUT_V2_INPUT_METHOD_CHANGED, serial, flags);
}

/**
 * zwp_text_input_manager_v2 - text input manager
 * @destroy: Destroy the wp_text_input_manager
 * @get_text_input: create a new text input object
 *
 * A factory for text-input objects. This object is a global singleton.
 */
struct zwp_text_input_manager_v2_interface {
	/**
	 * destroy - Destroy the wp_text_input_manager
	 *
	 * Destroy the wp_text_input_manager object.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * get_text_input - create a new text input object
	 * @id: (none)
	 * @seat: (none)
	 *
	 * Creates a new text-input object for a given seat.
	 */
	void (*get_text_input)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t id,
			       struct wl_resource *seat);
};


#ifdef  __cplusplus
}
#endif

#endif
