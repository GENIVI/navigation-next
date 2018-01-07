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

#ifndef TEXT_INPUT_UNSTABLE_V2_CLIENT_PROTOCOL_H
#define TEXT_INPUT_UNSTABLE_V2_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

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
 * @enter: enter event
 * @leave: leave event
 * @input_panel_state: state of the input panel
 * @preedit_string: pre-edit
 * @preedit_styling: pre-edit styling
 * @preedit_cursor: pre-edit cursor
 * @commit_string: commit
 * @cursor_position: set cursor to new position
 * @delete_surrounding_text: delete surrounding text
 * @modifiers_map: modifiers map
 * @keysym: keysym
 * @language: language
 * @text_direction: text direction
 * @configure_surrounding_text: configure amount of surrounding text to
 *	be sent
 * @input_method_changed: Notifies about a changed input method
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
struct zwp_text_input_v2_listener {
	/**
	 * enter - enter event
	 * @serial: serial to be used by update_state
	 * @surface: (none)
	 *
	 * Notification that this seat's text-input focus is on a certain
	 * surface.
	 *
	 * When the seat has the keyboard capability the text-input focus
	 * follows the keyboard focus.
	 */
	void (*enter)(void *data,
		      struct zwp_text_input_v2 *zwp_text_input_v2,
		      uint32_t serial,
		      struct wl_surface *surface);
	/**
	 * leave - leave event
	 * @serial: (none)
	 * @surface: (none)
	 *
	 * Notification that this seat's text-input focus is no longer on
	 * a certain surface.
	 *
	 * The leave notification is sent before the enter notification for
	 * the new focus.
	 *
	 * When the seat has the keyboard capabillity the text-input focus
	 * follows the keyboard focus.
	 */
	void (*leave)(void *data,
		      struct zwp_text_input_v2 *zwp_text_input_v2,
		      uint32_t serial,
		      struct wl_surface *surface);
	/**
	 * input_panel_state - state of the input panel
	 * @state: (none)
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * Notification that the visibility of the input panel (virtual
	 * keyboard) changed.
	 *
	 * The rectangle x, y, width, height defines the area overlapped by
	 * the input panel (virtual keyboard) on the surface having the
	 * text focus in surface local coordinates.
	 *
	 * That can be used to make sure widgets are visible and not
	 * covered by a virtual keyboard.
	 */
	void (*input_panel_state)(void *data,
				  struct zwp_text_input_v2 *zwp_text_input_v2,
				  uint32_t state,
				  int32_t x,
				  int32_t y,
				  int32_t width,
				  int32_t height);
	/**
	 * preedit_string - pre-edit
	 * @text: (none)
	 * @commit: (none)
	 *
	 * Notify when a new composing text (pre-edit) should be set
	 * around the current cursor position. Any previously set composing
	 * text should be removed.
	 *
	 * The commit text can be used to replace the composing text in
	 * some cases (for example when losing focus).
	 *
	 * The text input should also handle all preedit_style and
	 * preedit_cursor events occurring directly before preedit_string.
	 */
	void (*preedit_string)(void *data,
			       struct zwp_text_input_v2 *zwp_text_input_v2,
			       const char *text,
			       const char *commit);
	/**
	 * preedit_styling - pre-edit styling
	 * @index: (none)
	 * @length: (none)
	 * @style: (none)
	 *
	 * Sets styling information on composing text. The style is
	 * applied for length bytes from index relative to the beginning of
	 * the composing text (as byte offset). Multiple styles can be
	 * applied to a composing text by sending multiple preedit_styling
	 * events.
	 *
	 * This event is handled as part of a following preedit_string
	 * event.
	 */
	void (*preedit_styling)(void *data,
				struct zwp_text_input_v2 *zwp_text_input_v2,
				uint32_t index,
				uint32_t length,
				uint32_t style);
	/**
	 * preedit_cursor - pre-edit cursor
	 * @index: (none)
	 *
	 * Sets the cursor position inside the composing text (as byte
	 * offset) relative to the start of the composing text. When index
	 * is a negative number no cursor is shown.
	 *
	 * When no preedit_cursor event is sent the cursor will be at the
	 * end of the composing text by default.
	 *
	 * This event is handled as part of a following preedit_string
	 * event.
	 */
	void (*preedit_cursor)(void *data,
			       struct zwp_text_input_v2 *zwp_text_input_v2,
			       int32_t index);
	/**
	 * commit_string - commit
	 * @text: (none)
	 *
	 * Notify when text should be inserted into the editor widget.
	 * The text to commit could be either just a single character after
	 * a key press or the result of some composing (pre-edit). It could
	 * be also an empty text when some text should be removed (see
	 * delete_surrounding_text) or when the input cursor should be
	 * moved (see cursor_position).
	 *
	 * Any previously set composing text should be removed.
	 */
	void (*commit_string)(void *data,
			      struct zwp_text_input_v2 *zwp_text_input_v2,
			      const char *text);
	/**
	 * cursor_position - set cursor to new position
	 * @index: position of cursor
	 * @anchor: position of selection anchor
	 *
	 * Notify when the cursor or anchor position should be modified.
	 *
	 * This event should be handled as part of a following
	 * commit_string event.
	 *
	 * The text between anchor and index should be selected.
	 */
	void (*cursor_position)(void *data,
				struct zwp_text_input_v2 *zwp_text_input_v2,
				int32_t index,
				int32_t anchor);
	/**
	 * delete_surrounding_text - delete surrounding text
	 * @before_length: length of text before current cursor positon
	 * @after_length: length of text after current cursor positon
	 *
	 * Notify when the text around the current cursor position should
	 * be deleted. BeforeLength and afterLength is the length (in
	 * bytes) of text before and after the current cursor position
	 * (excluding the selection) to delete.
	 *
	 * This event should be handled as part of a following
	 * commit_string or preedit_string event.
	 */
	void (*delete_surrounding_text)(void *data,
					struct zwp_text_input_v2 *zwp_text_input_v2,
					uint32_t before_length,
					uint32_t after_length);
	/**
	 * modifiers_map - modifiers map
	 * @map: (none)
	 *
	 * Transfer an array of 0-terminated modifiers names. The
	 * position in the array is the index of the modifier as used in
	 * the modifiers bitmask in the keysym event.
	 */
	void (*modifiers_map)(void *data,
			      struct zwp_text_input_v2 *zwp_text_input_v2,
			      struct wl_array *map);
	/**
	 * keysym - keysym
	 * @time: (none)
	 * @sym: (none)
	 * @state: (none)
	 * @modifiers: (none)
	 *
	 * Notify when a key event was sent. Key events should not be
	 * used for normal text input operations, which should be done with
	 * commit_string, delete_surrounding_text, etc. The key event
	 * follows the wl_keyboard key event convention. Sym is a XKB
	 * keysym, state a wl_keyboard key_state. Modifiers are a mask for
	 * effective modifiers (where the modifier indices are set by the
	 * modifiers_map event)
	 */
	void (*keysym)(void *data,
		       struct zwp_text_input_v2 *zwp_text_input_v2,
		       uint32_t time,
		       uint32_t sym,
		       uint32_t state,
		       uint32_t modifiers);
	/**
	 * language - language
	 * @language: (none)
	 *
	 * Sets the language of the input text. The "language" argument
	 * is a RFC-3066 format language tag.
	 */
	void (*language)(void *data,
			 struct zwp_text_input_v2 *zwp_text_input_v2,
			 const char *language);
	/**
	 * text_direction - text direction
	 * @direction: (none)
	 *
	 * Sets the text direction of input text.
	 *
	 * It is mainly needed for showing input cursor on correct side of
	 * the editor when there is no input yet done and making sure
	 * neutral direction text is laid out properly.
	 */
	void (*text_direction)(void *data,
			       struct zwp_text_input_v2 *zwp_text_input_v2,
			       uint32_t direction);
	/**
	 * configure_surrounding_text - configure amount of surrounding
	 *	text to be sent
	 * @before_cursor: (none)
	 * @after_cursor: (none)
	 *
	 * Configure what amount of surrounding text is expected by the
	 * input method. The surrounding text will be sent in the
	 * set_surrounding_text request on the following state information
	 * updates.
	 */
	void (*configure_surrounding_text)(void *data,
					   struct zwp_text_input_v2 *zwp_text_input_v2,
					   int32_t before_cursor,
					   int32_t after_cursor);
	/**
	 * input_method_changed - Notifies about a changed input method
	 * @serial: serial to be used by update_state
	 * @flags: currently unused
	 *
	 * The input method changed on compositor side, which invalidates
	 * all current state information. New state information should be
	 * sent from the client via state requests (set_surrounding_text,
	 * set_content_hint, ...) and update_state.
	 */
	void (*input_method_changed)(void *data,
				     struct zwp_text_input_v2 *zwp_text_input_v2,
				     uint32_t serial,
				     uint32_t flags);
};

static inline int
zwp_text_input_v2_add_listener(struct zwp_text_input_v2 *zwp_text_input_v2,
			       const struct zwp_text_input_v2_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) zwp_text_input_v2,
				     (void (**)(void)) listener, data);
}

#define ZWP_TEXT_INPUT_V2_DESTROY	0
#define ZWP_TEXT_INPUT_V2_ENABLE	1
#define ZWP_TEXT_INPUT_V2_DISABLE	2
#define ZWP_TEXT_INPUT_V2_SHOW_INPUT_PANEL	3
#define ZWP_TEXT_INPUT_V2_HIDE_INPUT_PANEL	4
#define ZWP_TEXT_INPUT_V2_SET_SURROUNDING_TEXT	5
#define ZWP_TEXT_INPUT_V2_SET_CONTENT_TYPE	6
#define ZWP_TEXT_INPUT_V2_SET_CURSOR_RECTANGLE	7
#define ZWP_TEXT_INPUT_V2_SET_PREFERRED_LANGUAGE	8
#define ZWP_TEXT_INPUT_V2_UPDATE_STATE	9

static inline void
zwp_text_input_v2_set_user_data(struct zwp_text_input_v2 *zwp_text_input_v2, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zwp_text_input_v2, user_data);
}

static inline void *
zwp_text_input_v2_get_user_data(struct zwp_text_input_v2 *zwp_text_input_v2)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zwp_text_input_v2);
}

static inline void
zwp_text_input_v2_destroy(struct zwp_text_input_v2 *zwp_text_input_v2)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) zwp_text_input_v2);
}

static inline void
zwp_text_input_v2_enable(struct zwp_text_input_v2 *zwp_text_input_v2, struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_ENABLE, surface);
}

static inline void
zwp_text_input_v2_disable(struct zwp_text_input_v2 *zwp_text_input_v2, struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_DISABLE, surface);
}

static inline void
zwp_text_input_v2_show_input_panel(struct zwp_text_input_v2 *zwp_text_input_v2)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_SHOW_INPUT_PANEL);
}

static inline void
zwp_text_input_v2_hide_input_panel(struct zwp_text_input_v2 *zwp_text_input_v2)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_HIDE_INPUT_PANEL);
}

static inline void
zwp_text_input_v2_set_surrounding_text(struct zwp_text_input_v2 *zwp_text_input_v2, const char *text, int32_t cursor, int32_t anchor)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_SET_SURROUNDING_TEXT, text, cursor, anchor);
}

static inline void
zwp_text_input_v2_set_content_type(struct zwp_text_input_v2 *zwp_text_input_v2, uint32_t hint, uint32_t purpose)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_SET_CONTENT_TYPE, hint, purpose);
}

static inline void
zwp_text_input_v2_set_cursor_rectangle(struct zwp_text_input_v2 *zwp_text_input_v2, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_SET_CURSOR_RECTANGLE, x, y, width, height);
}

static inline void
zwp_text_input_v2_set_preferred_language(struct zwp_text_input_v2 *zwp_text_input_v2, const char *language)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_SET_PREFERRED_LANGUAGE, language);
}

static inline void
zwp_text_input_v2_update_state(struct zwp_text_input_v2 *zwp_text_input_v2, uint32_t serial, uint32_t reason)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_v2,
			 ZWP_TEXT_INPUT_V2_UPDATE_STATE, serial, reason);
}

#define ZWP_TEXT_INPUT_MANAGER_V2_DESTROY	0
#define ZWP_TEXT_INPUT_MANAGER_V2_GET_TEXT_INPUT	1

static inline void
zwp_text_input_manager_v2_set_user_data(struct zwp_text_input_manager_v2 *zwp_text_input_manager_v2, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zwp_text_input_manager_v2, user_data);
}

static inline void *
zwp_text_input_manager_v2_get_user_data(struct zwp_text_input_manager_v2 *zwp_text_input_manager_v2)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zwp_text_input_manager_v2);
}

static inline void
zwp_text_input_manager_v2_destroy(struct zwp_text_input_manager_v2 *zwp_text_input_manager_v2)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_text_input_manager_v2,
			 ZWP_TEXT_INPUT_MANAGER_V2_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) zwp_text_input_manager_v2);
}

static inline struct zwp_text_input_v2 *
zwp_text_input_manager_v2_get_text_input(struct zwp_text_input_manager_v2 *zwp_text_input_manager_v2, struct wl_seat *seat)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) zwp_text_input_manager_v2,
			 ZWP_TEXT_INPUT_MANAGER_V2_GET_TEXT_INPUT, &zwp_text_input_v2_interface, NULL, seat);

	return (struct zwp_text_input_v2 *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
