use crate::structures::{
    fried,
    raw::{ProfileRefTrait, ProfileTrait},
};
use std::slice::Iter;

pub struct ProfileList<'a, P: ProfileTrait> {
    pub list: &'a Vec<P>,
}

impl<'a, P: ProfileTrait> ProfileList<'a, P> {
    /// Get a profile by its `profile_name`
    pub fn get_profile(&self, profile_name: &str) -> &P {
        for profile in self.list {
            if profile_name == profile.get_pname() {
                return profile;
            }
        }
        // The function should always find a match, otherwise its an error
        panic!(
            "HostProfile named `{}` not found in raw-falafels file",
            profile_name
        );
    }
}

/// A ProfilesHandler lets us nicely spread Profiles using cycle iterators.
/// Profiles must implement ProfileTrait so we can get them by their name.
/// ProfilesReference must implement ProfileRefTrait so we can get the name of the profile
pub struct ProfilesCycler<'a, P: ProfileTrait, R: ProfileRefTrait> {
    // Defining cycle iterators to evenly allocate the profiles
    profiles_iter_trainers: std::iter::Cycle<Iter<'a, R>>,
    profiles_iter_aggregators: std::iter::Cycle<Iter<'a, R>>,
    profiles_list: ProfileList<'a, P>,
}

impl<'a, P: ProfileTrait, R: ProfileRefTrait> ProfilesCycler<'a, P, R> {
    /// Creates a new ProfilesCycler given `profiles_trainers` and `profiles_aggregators`
    /// which corresponds to the profiles we want to give to our trainers and aggregators.
    /// Lastly `profiles_list` is a vector of profiles implementing ProfileTrait.
    pub fn new(
        profiles_trainers: &'a Vec<R>,
        profiles_aggregators: &'a Vec<R>,
        profiles_list: ProfileList<'a, P>,
    ) -> ProfilesCycler<'a, P, R> {
        // Create a cycle operator from the profiles vector.
        let profiles_iter_trainers = profiles_trainers.iter().cycle();
        let profiles_iter_aggregators = profiles_aggregators.iter().cycle();

        let res: ProfilesCycler<P, R> = ProfilesCycler::<P, R> {
            profiles_iter_trainers,
            profiles_iter_aggregators,
            profiles_list,
        };
        res
    }

    /// Pick a profile depending on Node's Role
    pub fn pick_profile(&mut self, node_role: &fried::NodeRole) -> &P {
        match node_role {
            fried::NodeRole::Trainer(_) => {
                // Theoritically the cycle iterator never ends so its safe to unwrap.
                let profile_name = self.profiles_iter_trainers.next().unwrap();
                self.profiles_list.get_profile(profile_name.get_rname())
            }
            fried::NodeRole::Aggregator(_) => {
                let profile_name = self.profiles_iter_aggregators.next().unwrap();
                self.profiles_list.get_profile(profile_name.get_rname())
            }
        }
    }
}
