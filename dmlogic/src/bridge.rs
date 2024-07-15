use core::panic;

use ffi::*;
use crate::motherboard::{MotherboardEvent};

use crate::motherboard::{Motherboard, new_motherboard};

#[cxx::bridge(namespace = "bridge::dml")]
pub mod ffi {

    // Shared enums and structs with fields visible to both languages.
    #[derive(Clone)]
    pub enum RoleEnum {
        MainAggregator,
        Aggregator,
        Trainer,
    }

    #[derive(Clone)]
    pub struct NodeInfo {
        pub name: String,
        pub role: RoleEnum,
    }

    // Rust types and signatures exposed to C++.
    extern "Rust" {
        type Motherboard;
        // fn new_motherboard(node_info: NodeInfo) -> Box<Motherboard>;

        // // Wrapper to receive Tasks from the Motherboard
        // type TaskWrapper;
        // fn task_kind(self: &TaskWrapper) -> TaskKind;
        // fn get_send(self: &TaskWrapper) -> TaskSend;
        // fn get_exec(self: &TaskWrapper) -> TaskExec;

        // // Wrapper to send Events to the Motherboard
        // type EventWrapper;
        // fn new_event() -> Box<EventWrapper>;
        // fn set_receiv(self: &mut EventWrapper, event_send: EventReceiv);
        // fn set_exec(self: &mut EventWrapper, event_exec: EventEndExec);
    }

    // C++ types and signatures exposed to  Rust.
    unsafe extern "C++" {
        include!("dml.h");

        fn get_time() -> f64;
    }
}

// pub struct TaskWrapper {
//     pub task: MotherboardTask
// }
// 
// // TODO: can we handle this better? The constraints is that we are limited to C-like enums, that
// // why we need the wrapper
// impl TaskWrapper {
//     fn task_kind(self: &TaskWrapper) -> TaskKind {
//         match self.task {
//             MotherboardTask::Exec(_) => TaskKind::Exec,
//             MotherboardTask::Send(_) => TaskKind::Send,
//         }
//     }
// 
//     fn get_send(&self) -> TaskSend {
//         if let MotherboardTask::Send(t) = &self.task {
//             return t.clone();
//         } else {
//             panic!("Called get_send() on a Task that wasn't a send()")
//         }
//     }
// 
//     fn get_exec(&self) -> TaskExec {
//         if let MotherboardTask::Exec(t) = &self.task {
//             return t.clone();
//         } else {
//             panic!("Called get_send() on a Task that wasn't a send()")
//         }
//     }
// }
// 
// fn new_event() -> Box<EventWrapper> {
//     Box::new(EventWrapper { event: MotherboardEvent::NotInstanciated })
// }
// 
// pub struct EventWrapper {
//     pub event: MotherboardEvent
// }
// 
// impl EventWrapper {
//     fn set_receiv(&mut self, event_send: EventReceiv) {
//         self.event = MotherboardEvent::Receiv(event_send);
//     }
// 
//     fn set_exec(&mut self, event_exec: EventEndExec) {
//         self.event = MotherboardEvent::EndExec(event_exec);
//     }
// }
