from __future__ import annotations
import pyke3d as pyke
import numpy as np
import random
from _test_data import *

###############################################################################
# This example illustrates buffer update and resize
###############################################################################

class TestAp:
    def __init__(self):
        # Set up two camera specs using some common descriptors for
        # computer graphics cameras
        camera_specs_oc = pyke.vk_camera_specs(
            type=pyke.vk_camera_type.Rasterizer_IM,
            w_pos=np.array([5,5,5], np.float32),
            w_look=np.array([0,0,0], np.float32),
            w_up=np.array([0,0,1], np.float32),
            fow=80.0 / 180.0 * np.pi,
            w_near=1.0,
            w_far=100.0,
            steering_type=pyke.vk_steering_type.camera_centric # the camera turns around it's own center
        )
        print("Created camera 1 specs")

        camera_specs_cc = pyke.vk_camera_specs(
            type=pyke.vk_camera_type.Rasterizer_IM,
            w_pos=np.array([5,5,5], np.float32),
            w_look=np.array([0,0,0], np.float32),
            w_up=np.array([0,0,1], np.float32),
            fow=80.0 / 180.0 * np.pi,
            w_near=1.0,
            w_far=100.0,
            steering_type=pyke.vk_steering_type.object_centric # the camera turns around the point where it's aimed at
        )
        print("Created camera 2 specs")

        c_from = pyke.vk_rgb_color(r=0.5, g=0.0, b=0.0)
        c_to = pyke.vk_rgb_color(r=0.0, g=0.0, b=0.5)

        # crate a grid layout of size 3x3 with 5 pixels margin around each window
        # (pyke.vk_color_op.rgb_lerp interpolates color by transfering them to oklab space and back which 
        #  results in a very linear-looking change in colors)
        layout = pyke.vk_grid_layout(3, 3, 5, 5)
        layout.vk_add_camera(0,0, pyke.vk_layout_pack(camera_specs_oc, pyke.vk_color_op.rgb_lerp(0.000, c_from, c_to)))
        layout.vk_add_camera(0,1, pyke.vk_layout_pack(camera_specs_cc, pyke.vk_color_op.rgb_lerp(0.125, c_from, c_to)))
        layout.vk_add_camera(0,2, pyke.vk_layout_pack(camera_specs_oc, pyke.vk_color_op.rgb_lerp(0.250, c_from, c_to)))
        layout.vk_add_camera(1,0, pyke.vk_layout_pack(camera_specs_cc, pyke.vk_color_op.rgb_lerp(0.375, c_from, c_to)))
        layout.vk_add_camera(1,1, pyke.vk_layout_pack(camera_specs_oc, pyke.vk_color_op.rgb_lerp(0.500, c_from, c_to)))
        layout.vk_add_camera(1,2, pyke.vk_layout_pack(camera_specs_cc, pyke.vk_color_op.rgb_lerp(0.625, c_from, c_to)))
        layout.vk_add_camera(2,0, pyke.vk_layout_pack(camera_specs_oc, pyke.vk_color_op.rgb_lerp(0.750, c_from, c_to)))
        layout.vk_add_camera(2,1, pyke.vk_layout_pack(camera_specs_cc, pyke.vk_color_op.rgb_lerp(0.875, c_from, c_to)))
        layout.vk_add_camera(2,2, pyke.vk_layout_pack(camera_specs_oc, pyke.vk_color_op.rgb_lerp(1.000, c_from, c_to)))
        print("Added all cameras")

        # Create a device and a viewer.
        # The device represents the GPU. If multiple gpu's are available, indicate the preference.
        self.device = pyke.vk_device("TestName", device_preferences=pyke.vk_device_preferences.use_discrete_gpu)
        print("Created device")

        # crate a viewer using some parameterization for the window and a screenshot-save-path.
        # screenshots can be created by pressing ctrl+s
        v_params = pyke.vk_viewer_params(
            name="Pyke", 
            width=1024, 
            height=800, 
            viewing_type=pyke.vk_viewing_type.local)
        # pyke.vk_viewing_type.all => broadcast mouse movements on all cameras
        # pyke.vk_viewing_type.local => move camera that contains the mouse pointer on click
        print("Created viewer params")

        self.cam = pyke.vk_viewer(self.device, v_params)
        print("Created viewer")

        # add all cameras created inside the device layout. Cameras can also be added without a layout 
        # but there are only C++ examples available for it inside the test cases
        self.cam.vk_add_camera(layout.vk_layout_list(1024, 800))
        print("Added all cameras to viewer")

        # create a bunch of sample objects. Check out _test_data.py for how to create the data for them
        
        min_size = 1000
        max_size = 100000
        size = random.randint(min_size,max_size)
        self.random_dots = pyke.vk_dot_p_c(
            device=self.device,
            name="test_object",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]], dtype=np.float32),
            points=(2*np.random.rand(size,3)-1).astype(dtype=np.float32), 
            colors=np.random.rand(size,3).astype(dtype=np.float32), 
            indices=np.arange(size, dtype=np.int32),
            point_size=1,
            alpha=1.0,
            cull_mode=pyke.vk_cull_mode.none,
            # Buffer size behaviour
            # =====================
            # sizeBehaviour=pyke.vk_buffer_size_behaviour.init_1_0_grow_1_5, # init the buffer size with the exact data size, resize to 1.5*size if a resize is required
            sizeBehaviour=pyke.vk_buffer_size_behaviour.init_1_5_grow_1_5, # init the buffer size with 1.5*(data size), resize to 1.5*(data size) if a resize is required
            # Buffer update behaviour
            # =======================
            # updateBehaviour=pyke.vk_buffer_update_behaviour.global_lock # one buffer but global lock at data transfer
            updateBehaviour=pyke.vk_buffer_update_behaviour.double_buffering # two buffers on GPU at all times, global lock during switch, asynchronous data transfer
            # updateBehaviour=pyke.vk_buffer_update_behaviour.lazy_double_buffering # create a new buffer on update and switch at update time, global lock during switch, delete old buffer afterwards
            # updateBehaviour=pyke.vk_buffer_update_behaviour.pinned # use CPU accessible GPU memory for buffer, NOTE: this may cause the camera buffer to not work, depending on the GPU, it uses the same memory as UniformBuffers
        )

        # NOTE: CPU accessible GPU memory is used for things like UniformBuffers that contain camera matrix that is supposed to be updated on mouse movement.
        # If the .pinned strategy is used and updates take place without sleep, this will cause the camera movements to stall/delay/...
        # On Python it's difficult to produce updates quickly enough to get this effect. With C++, this is what happens without sleeping in a loop.

        print("Created test_object")

        # vk_run_thread starts the viewer and runs it inside a thread separate from the main thread.
        # Anywhere after this point, debug points will not interrupt the rendering process.
        self.cam.vk_run_thread()
        print("Started main thread")

        # add all the objects that were created. Note, that each object can also be attached to a single
        # camera or a subset of them
        self.cam.vk_attach_to_all(self.random_dots)
        print("Attached all objects to viewer")

        # After each modification of an object or attach/detach operation, thias one has to be called.
        # The only exception is if the data (for example points) don't change the size and format.
        # The method recreates the drawing commands for Vulkan. It has a very low overhead in the range of
        # microseconds.
        self.cam.vk_rebuild_and_redraw()
        print("Rebuild viewer draw commands")

        # This while loop is responsible for keeping the program from terminating early.
        while(self.cam.vk_running()):
            ####################################################################
            # This is the spot where processing code can be added. The viewer
            # window runs inside a separate thread and will not block.
            # NOTE: This causes the window mechanisms to block: with 1 second
            # sleep, it will take between 0 and 1 seconds to close the main window!
            # NOTE: the first time this runs, it will likely print a warning that the buffer
            # has been resized. This is intended behaviour
            ####################################################################
            size = random.randint(min_size, max_size)
            self.random_dots.vk_update_points((2*np.random.rand(size,3)-1).astype(dtype=np.float32), 0)
            self.random_dots.vk_update_colors(np.random.rand(size,3).astype(dtype=np.float32), 0)
            self.random_dots.vk_update_indices(np.arange(size, dtype=np.int32), 0)
            self.cam.vk_rebuild_and_redraw()

        # Detach all the objects from the cameras
        # (Technically not necessary. The destructors should take care of it)
        self.cam.vk_detach_from_all(self.random_dots)
        print("Detached all objects from viewer")

if __name__ == "__main__":
    p = TestAp()
