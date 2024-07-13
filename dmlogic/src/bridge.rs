use ffi::*;
use crate::protocol::{Event, Task};

use crate::motherboard::{Motherboard, new_motherboard};

#[cxx::bridge(namespace = "bridge::dml")]
pub mod ffi {


    // Shared structs with fields visible to both languages.
    struct EventEndExec {
        yep: u16,
    }

    struct EventEndSend {
        yep: u16,
    }   

    pub struct TaskExec {
        yep: u16,
    }

    pub struct TaskSend {
        yep: u16,
    }

    pub struct Bonsoir {
        exemple: String
    }

    // Rust types and signatures exposed to C++.
    extern "Rust" {
        type Motherboard;
        fn new_motherboard() -> Box<Motherboard>;

        type TaskWrapper;
        fn new_task() -> Box<TaskWrapper>;
        fn set_send(self: &mut TaskWrapper, task_send: TaskSend);
        fn set_exec(self: &mut TaskWrapper, task_exec: TaskExec);

        type EventWrapper;
        fn new_event() -> Box<EventWrapper>;
        fn set_send(self: &mut EventWrapper, event_send: EventEndSend);
        fn set_exec(self: &mut EventWrapper, event_exec: EventEndExec);
    }

    // C++ types and signatures exposed to  Rust.
    unsafe extern "C++" {
        include!("dml.h");

        fn get_time() -> f64;
    }
}


fn new_task() -> Box<TaskWrapper> {
    Box::new(TaskWrapper { task: Task::NotInstanciated })
}

pub struct TaskWrapper {
    task: Task
}

impl TaskWrapper {
    fn set_send(&mut self, task_send: TaskSend) {
        self.task = Task::Send(task_send);
    }

    fn set_exec(&mut self, task_exec: TaskExec) {
        self.task = Task::Exec(task_exec);
    }
}

fn new_event() -> Box<EventWrapper> {
    Box::new(EventWrapper { event: Event::NotInstanciated })
}

pub struct EventWrapper {
    event: Event
}

impl EventWrapper {
    fn set_send(&mut self, event_send: EventEndSend) {
        self.event = Event::EndSend(event_send);
    }

    fn set_exec(&mut self, event_exec: EventEndExec) {
        self.event = Event::EndExec(event_exec);
    }
}
