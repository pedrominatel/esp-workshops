# This script prepares the sensor dataset for training a 1D CNN model to recognize hand gestures.
# It reads the raw CSV files, processes the data, creates a 1D CNN model, trains it, and saves the trained model in H5 format for later conversion to TFLite

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from tensorflow import keras


def extract_data():
    # Read data files
    o_data = pd.read_csv('./shuttle_dataset/o.csv', sep=',', header=None)
    v_data = pd.read_csv('./shuttle_dataset/v.csv', sep=',', header=None)
    unknown_data = pd.read_csv('./shuttle_dataset/unknown.csv', sep=',', header=None)
    # Add the new dataset for "right" gesture or any other gesture you want to include
    right_data = pd.read_csv('./shuttle_dataset/right.csv', sep=',', header=None)

    # Create labels
    o_label = np.zeros(o_data.shape[0], dtype=int)
    v_label = np.ones(v_data.shape[0], dtype=int)
    unknown_label = np.full(unknown_data.shape[0], 2, dtype=int)
    # Add labels for the new "right" gesture (label 3) - This will be used in the application and you need to add on app_model.cpp as well (kLabels[])
    right_label = np.full(right_data.shape[0], 3, dtype=int)

    # Combine feature data and labels (use .values for all to ensure numpy arrays)
    X_raw = np.vstack([o_data.values, v_data.values, unknown_data.values, right_data.values])
    y = np.concatenate([o_label, v_label, unknown_label, right_label])

    # Display the number of samples for each class
    print(f"o samples: {o_label.shape[0]}")
    print(f"v samples: {v_label.shape[0]}")
    print(f"unknown samples: {unknown_label.shape[0]}")
    # Add print statement for the new "right" gesture samples
    print(f"right samples: {right_label.shape[0]}")

    print(f"Total samples: {len(y)}")

    num_samples = X_raw.shape[0]
    num_timesteps = 200  # 200 timesteps
    num_axes = 3  # x, y, z three axes

    # Reshape data: reshape 600 data points in each row into 200x3 matrix
    X = X_raw.reshape(num_samples, num_timesteps, num_axes)

    return X, y


def create_1d_cnn_model():
    model = keras.Sequential([
        keras.layers.Conv1D(filters=8, kernel_size=5, padding='same', activation='relu', input_shape=(200, 3)),
        keras.layers.MaxPooling1D(pool_size=4),

        keras.layers.Conv1D(filters=16, kernel_size=5, padding='same', activation='relu'),
        keras.layers.MaxPooling1D(pool_size=4),

        keras.layers.GlobalAveragePooling1D(),

        keras.layers.Dense(32, activation='relu'),
        keras.layers.Dropout(0.2),

        # Final output layer with 4 classes (o, v, unknown, right) - If you add or remove, update the layers number
        keras.layers.Dense(4, activation='softmax')
    ])

    return model


def train_model(model, X_train, y_train, X_test, y_test, epochs=30):
    """
    Train the model using Keras fit method
    """
    # Compile the model
    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=0.001),
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )

    # Print model summary
    print("Model Summary:")
    model.summary()

    # Train the model
    history = model.fit(
        X_train, y_train,
        validation_data=(X_test, y_test),
        epochs=epochs,
        batch_size=64,
        verbose=1,
        shuffle=True
    )

    return history


def plot_training_curves(history, save_path="training_curves.png"):
    """
    Plot training loss and accuracy curves (train + validation)
    """
    fig, axes = plt.subplots(1, 2, figsize=(12, 4))

    epochs = range(1, len(history.history['loss']) + 1)

    # Loss
    axes[0].plot(epochs, history.history['loss'], 'b-', label='Train Loss')
    axes[0].plot(epochs, history.history['val_loss'], 'r-', label='Val Loss')
    axes[0].set_xlabel('Epoch')
    axes[0].set_ylabel('Loss')
    axes[0].set_title('Training and Validation Loss')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)

    # Accuracy
    axes[1].plot(epochs, history.history['accuracy'], 'b-', label='Train Acc')
    axes[1].plot(epochs, history.history['val_accuracy'], 'r-', label='Val Acc')
    axes[1].set_xlabel('Epoch')
    axes[1].set_ylabel('Accuracy')
    axes[1].set_title('Training and Validation Accuracy')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(save_path, dpi=150)
    print(f"\nTraining curves saved to {save_path}")
    plt.show()


def save_model_h5(model, filepath):
    """
    Save the model in H5 format
    """
    model.save(filepath)
    print(f"Model saved to {filepath}")


if __name__ == "__main__":
    print("\n=== Data Extraction Phase ===")
    X, y = extract_data()
    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.int32)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)
    print(f"Training set: {X_train.shape[0]} samples, Test set: {X_test.shape[0]} samples")

    print("\n=== Model Creation ===")
    model = create_1d_cnn_model()

    print("\n=== Starting Training ===")
    history = train_model(model, X_train, y_train, X_test, y_test, epochs=300)

    print("\n=== Saving Model ===")
    save_model_h5(model, "simple_1dcnn_model.h5")
    print("Training completed!")