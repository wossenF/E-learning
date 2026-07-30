import * as crypto from 'crypto';

export class MediaTokenUtils {
  private static readonly MEDIA_SECRET = process.env.MEDIA_STREAM_SECRET || 'temene-media-stream-secure-token-signing-key-32chars';

  /**
   * Generates a time-limited signed URL token for streaming media content.
   */
  public static generateStreamToken(assetId: string, userId: string, ttlSeconds = 3600): { token: string; expiresAt: number } {
    const expiresAt = Math.floor(Date.now() / 1000) + ttlSeconds;
    const payload = `${assetId}:${userId}:${expiresAt}`;
    const hash = crypto.createHmac('sha256', this.MEDIA_SECRET).update(payload).digest('hex');
    const token = `${Buffer.from(payload).toString('base64url')}.${hash}`;

    return { token, expiresAt };
  }

  /**
   * Verifies a signed media stream token.
   */
  public static verifyStreamToken(tokenStr: string, assetId: string): boolean {
    if (!tokenStr || !tokenStr.includes('.')) return false;

    const [b64Payload, signature] = tokenStr.split('.');
    try {
      const payload = Buffer.from(b64Payload, 'base64url').toString('utf8');
      const parts = payload.split(':');
      if (parts.length !== 3) return false;

      const [tokenAssetId, userId, expiresAtStr] = parts;
      if (tokenAssetId !== assetId) return false;

      const expiresAt = parseInt(expiresAtStr, 10);
      if (Math.floor(Date.now() / 1000) > expiresAt) return false;

      const expectedHash = crypto.createHmac('sha256', this.MEDIA_SECRET).update(payload).digest('hex');
      return crypto.timingSafeEqual(Buffer.from(expectedHash, 'hex'), Buffer.from(signature, 'hex'));
    } catch {
      return false;
    }
  }
}
