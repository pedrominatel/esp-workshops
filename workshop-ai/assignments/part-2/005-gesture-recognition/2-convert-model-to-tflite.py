# This script converts the trained Keras model to TensorFlow Lite format for deployment on embedded devices.

import tensorflow as tf
from tensorflow import keras

# Load the trained Keras model from the previous step
model = keras.models.load_model("./simple_1dcnn_model.h5")
converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.float32]
converter.inference_input_type = tf.float32
converter.inference_output_type = tf.float32

tflite_model = converter.convert()

with open("./model.tflite", "wb") as f:
    f.write(tflite_model)