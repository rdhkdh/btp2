import tensorflow as tf
from tensorflow.keras.layers import Input, Conv1D, Dense, Dropout, BatchNormalization, ReLU, Attention, GlobalAveragePooling1D, Concatenate, Flatten, Softmax, MultiHeadAttention, GaussianNoise
from tensorflow.keras.models import Model
from tensorflow.keras.regularizers import l2

# parameters
learning_rate = 0.025
dropout_rate = 0.4

# Convolutional block
def conv_block(inputs, filters, kernel_size, stride, regularization):
    x = Conv1D(filters=filters, kernel_size=kernel_size, strides=stride, padding="same", activation=None, kernel_regularizer=l2(regularization))(inputs)
    x = Dropout(dropout_rate)(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    return x

# Attention Block
def attention_block(inputs, n_heads, key_dim, regularization):
    x = MultiHeadAttention(num_heads=n_heads, key_dim=key_dim, kernel_regularizer=l2(regularization))(inputs, inputs)
    x = Dropout(dropout_rate)(x)
    x = BatchNormalization()(x)
    #x = GlobalAveragePooling1D()(x)
    return x

# Dense Block
def dense_block(inputs, units, noise_stddev, regularization):
    x = Dense(units, kernel_regularizer=l2(regularization))(inputs)
    x = GaussianNoise(stddev=noise_stddev)(x)  # Add Gaussian Noise
    x = Dropout(dropout_rate)(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    return x


# defines a HEAD of the mhdnn
def head(input):
    f = conv_block(input, 8, 6, 2, 1e-6)  #ip:(None, 14000, 2), op:(None, 7000, 8)
    f = conv_block(f, 16, 6, 1, 1e-6)     #ip:(None, 7000, 8), op:(None, 7000, 16)
    f = conv_block(f, 16, 5, 2, 1e-6)     #ip:(None, 7000, 16), op:(None, 3500, 16)
    a = attention_block(f, 8, 8, 1e-5)    #ip:(None, 3500, 16), op:(None, 3500, 16)---removed global avg pooling
    return a

# defines BODY of the mhdnn
def body(input):
    f = conv_block(input, 8, 3, 1, 1e-6)  #ip:(None, 3500, 32), op:(None, 3500, 8)
    f = conv_block(f, 16, 2, 1, 1e-6)     #ip:(None, 3500, 8), op:(None, 3500, 16)
    f = conv_block(f, 16, 2, 1, 1e-6)     #ip:(None, 3500, 16), op:(None, 3500, 16)
    f = Flatten()(f)                      #ip:(None, 3500, 16), op:(None, 56000)
    f = dense_block(f, 100, 0.3, 1e-5)    #ip:(None, 3500, 16), op:(None, 100)
    f = dense_block(f, 50, 0, 1e-5) # no gaussian noise here         #ip:(None, 100), op:(None, 50)
    # Output Layer with Softmax
    output = Dense(2, activation="softmax", name="output")(f)         #ip:(None, 50), op:(None, 2)
    return output

# Multi-Head Deep Neural Network (MH-DNN) Model
def build_mhdnn():
    input_shape = (14000, 2)
    input_rssi = Input(shape=input_shape, name="rssi_input") 
    input_sinr = Input(shape=input_shape, name="sinr_input")

    # pass through respective head
    rssi_attn = head(input_rssi)
    sinr_attn = head(input_sinr)

    # Ensure shapes are correct before concatenation
    print("RSSI head output shape:", rssi_attn.shape)  # Debug
    print("SINR head output shape:", sinr_attn.shape)  # Debug

    # Concatenate extracted features
    merged_features = Concatenate(axis=-1)([rssi_attn, sinr_attn])  #op:(None, 3500, 32)

    print("Merged features shape:", merged_features.shape)  # Debug

    # pass through body of mhdnn
    output = body(merged_features)

    # Build model
    model = Model(inputs=[input_rssi, input_sinr], outputs=output, name="MH-DNN")
    return model