// Copyright Antony Polukhin, 2016.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/function.hpp>
struct events {
    typedef boost::function<void (float)> callback_type;

    static void from_keyboard(callback_type callback);
    static void from_network(callback_type callback);
};


#include <boost/stacktrace.hpp>
#include <iostream>
BOOST_NOINLINE void validate_positive(float f) {
    if (f < 0.f) {
        std::cerr << "Negative number " << f << " detected. Call stack:\n"
            << boost::stacktrace::stacktrace() << '\n';
    }
}




boost::function<void (float)> on_keyboard;
boost::function<void (float)> on_network;

BOOST_NOINLINE void keyboard_event() {
    on_keyboard(-0.1f);
}

BOOST_NOINLINE void network_event() {
    on_network(1.0f);
}

void events::from_keyboard(events::callback_type callback) {
    on_keyboard = callback;
}

void events::from_network(events::callback_type callback) {
    on_network = callback;
}



int main() {
    events::from_keyboard(&validate_positive);
    events::from_network(&validate_positive);

    keyboard_event();
    network_event();
}



