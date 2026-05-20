import numpy as np
import torch
import torch.nn as nn


class Conv1d(nn.Module):
    def __init__(
        self,
        in_channels: int,
        out_channels: int,
        kernel_size: int,
        dilation: int,
    ):
        super().__init__()
        self.in_channels = in_channels
        self.out_channels = out_channels
        self.kernel_size = kernel_size
        self.dilation = dilation

        self.conv = nn.Conv1d(
            in_channels=in_channels,
            out_channels=out_channels,
            kernel_size=kernel_size,
            dilation=dilation,
            bias=True,
        )
        self.ring_buffer_length = (kernel_size - 1) * dilation

    def reset_state(
        self,
        batch_size: int,
        dtype: torch.dtype,
        device: torch.device,
    ):
        self.batch_size = batch_size
        if self.kernel_size > 1:
            self.ring_buffer = torch.zeros(
                batch_size,
                self.conv.in_channels,
                self.ring_buffer_length,
                dtype=dtype,
                device=device,
            )
        else:
            self.ring_buffer = None


    def detach_state(self):
        if self.ring_buffer is not None:
            self.ring_buffer = self.ring_buffer.detach()

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        if x.ndim != 3:
            raise ValueError("Input tensor must be 3D (batch_size, in_channels, length).")
        batch_size, in_channels, length = x.shape
        if batch_size != self.batch_size:
            raise ValueError("Batch size mismatch.")
        if in_channels != self.conv.in_channels:
            raise ValueError("Input channel mismatch.")
        if self.kernel_size > 1:
            # Update ring buffer
            x = torch.cat([self.ring_buffer, x], dim=2)

        y = self.conv(x)

        if self.kernel_size > 1:
            # Update ring buffer
            self.ring_buffer = x[:, :, -self.ring_buffer_length :]
        return y


class WaveNet(nn.Module):
    def __init__(
        self,
        num_embeddings: int,
        embedding_channels: int,
        hidden_channels: int,
        layers_per_stack: int,
        num_stacks: int,
        kernel_size: int,
    ):
        super().__init__()
        # Map input to hidden channels
        self.embedding_in = nn.Embedding(num_embeddings=num_embeddings, embedding_dim=embedding_channels)

        layers = []
        for i in range(num_stacks):
            for j in range(layers_per_stack):
                dilation = 2**j
                layers.append(
                    Conv1d(
                        in_channels=hidden_channels,
                        out_channels=hidden_channels,
                        kernel_size=kernel_size,
                        dilation=dilation,
                    )
                )
        self.layers = nn.ModuleList(layers)

        self.num_layers = num_stacks * layers_per_stack

        # output layer
        self.conv_out = Conv1d(in_channels=hidden_channels * self.num_layers, out_channels=num_embeddings, kernel_size=1, dilation=1)

    def reset_state(self, batch_size: int, dtype: torch.dtype, device: torch.device):
        for layer in self.layers:
            layer.reset_state(batch_size, dtype, device)
        self.conv_out.reset_state(batch_size, dtype, device)

    def detach_state(self):
        for layer in self.layers:
            layer.detach_state()
        self.conv_out.detach_state()

    def store_to_binary(self, file_path: str = "wavenet_weights.bin"):
        def conv_weight_cpp_flat(w: torch.Tensor) -> np.ndarray:
            """PyTorch Conv1d weight (out, in, kernel) -> C++ layout [kernel][out][in] row-major."""
            return np.transpose(w.cpu().detach().numpy(), (2, 0, 1)).flatten()

        all_weights = []
        all_weights.append(self.embedding_in.weight.cpu().detach().numpy().flatten())
        for i in range(self.num_layers):
            conv_kernel = conv_weight_cpp_flat(self.layers[i].conv.weight)
            conv_bias = self.layers[i].conv.bias.cpu().detach().numpy().flatten()
            all_weights.append(conv_kernel)
            all_weights.append(conv_bias)
        conv_out_kernel = conv_weight_cpp_flat(self.conv_out.conv.weight)
        conv_out_bias = self.conv_out.conv.bias.cpu().detach().numpy().flatten()
        all_weights.append(conv_out_kernel)
        all_weights.append(conv_out_bias)
        all_weights = np.concatenate(all_weights)
        all_weights = all_weights.astype(np.float32)
        all_weights.tofile(file_path)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """ Input shape: (batch_size, length) """
        x = self.embedding_in(x)  # (batch_size, length, embedding_channels)
        # transpose for Conv1d
        x = x.transpose(1, 2)  # (batch_size, embedding_channels, length)

        skips = []
        for i in range(self.num_layers):
            x = x + torch.tanh(self.layers[i](x))
            skips.append(x)
        skips = torch.cat(skips, dim=1)
        y = self.conv_out(skips)

        return y
