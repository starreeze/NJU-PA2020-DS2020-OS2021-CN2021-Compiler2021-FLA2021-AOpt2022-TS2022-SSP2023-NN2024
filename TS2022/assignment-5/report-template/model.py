class AttnModel(nn.Module):
    hidden_dim = 512
    num_heads = 4
    dropout = 0.5

    def __init__(self, configs):
        super(AttnModel, self).__init__()
        self.seq_len = configs.seq_len
        self.pred_len = configs.pred_len
        self.channels = configs.enc_in
        self.individual = configs.individual
        self.attention = nn.MultiheadAttention(
            self.hidden_dim, self.num_heads, self.dropout, batch_first=True
        )
        self.Linear1 = nn.Linear(self.seq_len, self.hidden_dim)
        self.activation1 = nn.GELU()
        self.Linear2 = nn.Linear(self.hidden_dim, self.pred_len)
        self.layernorm = nn.LayerNorm([self.channels, self.hidden_dim])

    def forward(self, x):
        # x: [Batch, Input length, Channel]
        seq_last = x[:, -1:, :].detach()
        x = x - seq_last
        x = self.activation1(self.Linear1(x.permute(0, 2, 1)))
        x, _ = self.attention(x, x, x)
        x = self.layernorm(x)
        x = self.Linear2(x).permute(0, 2, 1)
        x = x + seq_last
        return x  # [Batch, Output length, Channel]
