from __future__ import annotations
import pyke
import numpy as np
import os
from _test_data import *
import time

# =================================================================================================
# All callback functions must be placed outside of any class. Nanobind doesn't seem to work (yet?)
# with nb::callable that is bound to an object.
# Solution: use 'self:TestAp', using type annotations for auto-completion as first argument for
# the same behaviour
# =================================================================================================

# Demo on how to print the current camera configuration. There is no other way to get it currently.
# This information can be used to adapt the initial camera positions in the vk_camera_specs.
def cam_specs(self:TestAp, repeat):
    specs = self.cam.vk_camera_coords(0)
    print()
    print(specs.w_pos)
    print(specs.w_look)
    print(specs.w_up)
    print(specs.x_axis)
    print(specs.y_axis)
    print(specs.z_axis)

# attach and detach objects at runtime
def attach(self:TestAp, repeat):
    if self.dot.vk_is_attached_to(0):
        self.cam.vk_detach_from(0, self.dot)
    else:
        self.cam.vk_attach_to(0, self.dot)
    self.cam.vk_rebuild_and_redraw()

def incr_p_size(self:TestAp, repeat):
    self.point_size+=1
    self.dot.vk_update_point_size(self.point_size)
    self.cam.vk_rebuild_and_redraw()

def decr_p_size(self:TestAp, repeat):
    self.point_size -= 1
    if self.point_size < 1:
        self.point_size = 1
    self.dot.vk_update_point_size(self.point_size)
    self.cam.vk_rebuild_and_redraw()

def decr_alpha(self:TestAp, repeat):
    self.alpha -= 0.1
    if self.alpha < 0.0:
        self.alpha = 0.0
    self.dot.vk_update_alpha(self.alpha)
    self.cam.vk_rebuild_and_redraw()

def incr_alpha(self:TestAp, repeat):
    self.alpha += 0.1
    if self.alpha > 1.0:
        self.alpha = 1.0
    self.dot.vk_update_alpha(self.alpha)
    self.cam.vk_rebuild_and_redraw()

def onoff(self:TestAp, repeat):
    self.on = not self.on

def rotate(self:TestAp, repeat):
    self.angle += 1.0
    if(self.angle >= 360.0):
        self.angle -= 360.0
    self.dot.vk_update_points(TestData.Point_P(self.angle), 0)
    self.cam.vk_rebuild_and_redraw()

    if(self.on):
        repeat()

# The repeat parameter is a wrapped call to the function (in this case scale) itself.
# Calling repeat() will in this case call scale(repeat) again. Note that the call is not
# recursive. It simply dumps another call to this method inside a thread pool.
def scale(self:TestAp, repeat):
    if self.size >= 1.5:
        self.step = -0.01
    elif self.size <= 0.5:
        self.step = 0.01

    self.size += self.step
    self.dot.vk_update_model_matrix(model_matrix=np.array([[self.size,0,0,0],[0,self.size,0,0],[0,0,self.size,0],[1.5,1.5,0,1]], np.float32))
    self.cam.vk_rebuild_and_redraw()

    if(self.on):
        repeat()

class TestAp:
    def __init__(self):
        # Set up some camera specs using some common descriptors for
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
        print("Created viewer params")

        self.cam = pyke.vk_viewer(self.device, v_params)
        print("Created viewer")

        # add all cameras created inside the device layout. Cameras can also be added without a layout 
        # but there are only C++ examples available for it inside the test cases
        self.cam.vk_add_camera(layout.vk_layout_list(1024, 800))
        print("Added all cameras to viewer")

        # register some actions
        self.cam.vk_register_action(self, pyke.lwws_key.RControl, cam_specs)
        self.cam.vk_register_action(self, "r", rotate)
        self.cam.vk_register_action(self, "o", onoff)
        self.cam.vk_register_action(self, "s", scale)
        self.cam.vk_register_action(self, "p", decr_p_size)
        self.cam.vk_register_action(self, "i", incr_p_size)
        self.cam.vk_register_action(self, "z", decr_alpha)
        self.cam.vk_register_action(self, "x", incr_alpha)
        self.cam.vk_register_action(self, "a", attach)
        print("Registered all actions")

        self.point_size = 10.0
        self.line_width = 5.0
        self.alpha = 0.5
        self.size = 1.0
        self.step = 0.01
        self.angle = 0

        # create a bunch of sample objects. Check out _test_data.py for how to create the data for them
        self.dot = pyke.vk_dot_p_c(
            device=self.device,
            name="test_object",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[1.5,1.5,0,1]], dtype=np.float32),
            points=TestData.Point_P(self.angle), 
            colors=TestData.Point_C(), 
            indices=TestData.Point_P_C_Indices(),
            point_size=self.point_size,
            alpha=self.alpha,
            cull_mode=pyke.vk_cull_mode.none,
            sizeBehaviour=pyke.vk_buffer_size_behaviour.init_1_0_grow_1_5,
            updateBehaviour=pyke.vk_buffer_update_behaviour.global_lock
        )
        print("Created test_object")

        self.line = pyke.vk_line_p_c(
            device=self.device,
            name="line_obj",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[-1.5,1.5,0,1]], np.float32),
            points=TestData.Line_P(0.0),
            colors=TestData.Line_C(),
            indices=TestData.Line_P_C_Indices(),
            line_width=self.line_width,
            alpha=1.0
        )
        print("Created line_obj")

        f=-4.0
        t= 4.0
        l= 0.5
        self.coords = pyke.vk_line_p_c(
            device=self.device,
            name="coords",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]], np.float32),
            points=TestData.Coords_P(f,t,l, f,t,l, f,t,l),
            colors=TestData.Coords_C(1.0, 1.0, 1.0),
            indices=TestData.Coords_P_C_Indices(),
            line_width=2.0,
            alpha=1.0
        )
        print("Created coords")

        self.mesh = pyke.vk_mesh_p_c(
            device=self.device,
            name="mesh",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[-1.5,-1.5,0,1]], np.float32),
            points=TestData.Cube1_P(0.0),
            colors=TestData.Cube1_C(),
            indices=TestData.Cube1_P_C_Indices(),
            alpha=1.0,
            cull_mode=pyke.vk_cull_mode.back,
            render_type=pyke.vk_render_type.solid,
            line_width=1.0,
            point_size=1.0
        )
        print("Created mesh")

        self.mesh2 = pyke.vk_mesh_p_c(
            device=self.device,
            name="mesh2",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[1.5,-1.5,0,1]], np.float32),
            points=TestData.Cube2_P(0.0),
            colors=TestData.Cube2_C(),
            indices=TestData.Cube2_P_C_N_Indices(),
            alpha=1.0,
            cull_mode=pyke.vk_cull_mode.none,
            render_type=pyke.vk_render_type.wireframe,
            line_width=1.0,
            point_size=1.0
        )
        print("Created mesh2")

        self.mesh2_normals = pyke.vk_line_p_c(
            device=self.device,
            name="mesh2_normals",
            model_matrix=np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[1.5,-1.5,0,1]], np.float32),
            points=TestData.Cube2_NormalLines_P(0.5, 0.0),
            colors=TestData.Cube2_NormalLines_C(),
            indices=TestData.Cube2_NormalLines_Indices(),
            line_width=2.0,
            alpha=1.0
        )
        print("Created mesh2_normals")

        self.on = False

        # vk_run_thread starts the viewer and runs it inside a thread separate from the main thread.
        # Anywhere after this point, debug points will not interrupt the rendering process.
        self.cam.vk_run_thread()
        print("Started main thread")

        # add all the objects that were created. Note, that each object can also be attached to a single
        # camera or a subset of them
        self.cam.vk_attach_to_all(self.dot)
        self.cam.vk_attach_to_all(self.line)
        self.cam.vk_attach_to_all(self.coords)
        self.cam.vk_attach_to_all(self.mesh)
        self.cam.vk_attach_to_all(self.mesh2)
        self.cam.vk_attach_to_all(self.mesh2_normals)
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
            ####################################################################
            print("Just slept for 1 second")
            time.sleep(1)

        # Detach all the objects from the cameras
        # (Technically not necessary. The destructors should take care of it)
        self.cam.vk_detach_from_all(self.dot)
        self.cam.vk_detach_from_all(self.line)
        self.cam.vk_detach_from_all(self.coords)
        self.cam.vk_detach_from_all(self.mesh)
        self.cam.vk_detach_from_all(self.mesh2)
        self.cam.vk_detach_from_all(self.mesh2_normals)
        print("Detached all objects from viewer")

if __name__ == "__main__":
    p = TestAp()
