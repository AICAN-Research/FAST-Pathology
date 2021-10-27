from tensorflow.python.keras.models import load_model


model = load_model("./pw_tumour_mobilenetv2_model.h5", compile=False)
#model = load_model("./unet_tumour_refinement_updated.h5", compile=False)


print([node.op.name for node in model.inputs])
print([node.op.name for node in model.outputs])
