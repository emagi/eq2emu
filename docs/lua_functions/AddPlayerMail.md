### Function: AddPlayerMail(Spawn, FromName, SubjectName, MailBody, MailType, Copper, Silver, Gold, Platinum, ItemID, StackSize, ExpireTime, SentTime)

**Description:**
Sends a mail message to the Spawn.

**Parameters:**
- `Spawn`: Spawn - The Spawn (Player) to send the mail.
- `FromName`: string - From header name.
- `SubjectName`: string - Subject header name.
- `MailBody`: string - Body value.
- `MailType`: UInt8 - Type of mail, 0 = Regular, 1 = Spam, 2 = GM.
- `Copper`: UInt32 - Amount of copper included in the mail.
- `Silver`: UInt32 - Amount of silver included in the mail.
- `Gold`: UInt32 - Amount of gold included in the mail.
- `Platinum`: UInt32 - Amount of platinum included in the mail.
- `ItemID`: UInt32 - ItemID to provide to the Spawn in the mail.
- `StackSize`: UInt16 - StackSize (Quantity) of the item.
- `ExpireTime`: UInt32 - Unix Timestamp Expiration (defaults to 30 days after the sent time).  Not active?
- `SentTime`: UInt32 - Unix Timestamp of when the mail was sent (optional defaults to current time).

**Returns:** Boolean: True if successful adding mail, otherwise False.

**Example:**

```lua
-- Example usage: Sends Spawn a message from "Bob" with 1 Platinum and ItemID 1696 (a fishman scale amulet)
AddPlayerMail(Spawn, "Bob", "Mail for you", "What do you need, but some amulet?", 0, 0, 0, 0, 1, 1696)
```
