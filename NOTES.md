
### Frame resources.

```cpp
// resources are created using handles
const auto handle = graph.create_buffer();

// must have predefined resources for swapchain rt (?) maybe
const auto render_target = graph.get_final_rt();
// or use something else. like a struct to define this
// maybe RenderGraph::final_rt_handle;

struct PipelineNode  {
    vector<handle> inputs;
    vector<handle> outputs; // this can be just targets(?)
};
```


#### Event system todos are marked:
- // @TODO-EVENT_SYSTEM
